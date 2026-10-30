/******************************************************************************
 *  Copyright 2021 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#include <mutex>
#include <chrono>
#include "pn7160_diag.h"
#include <errno.h>
#include <fcntl.h>
#ifdef ANDROID
#include <hardware/nfc.h>
#endif
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <NfccAltI2cTransport.h>
#include <phNfcStatus.h>
#include <phNxpLog.h>
#include <string.h>
#include "phNxpNciHal_utils.h"

extern phTmlNfc_Context_t* gpphTmlNfc_Context;

#ifndef PN7160_ENABLE_DIAG
#define PN7160_ENABLE_DIAG 1
#endif
namespace {
std::mutex trace_mutex;
uint64_t trace_entries[256] = {};
uint32_t trace_next = 0;
uint32_t trace_used = 0;
}
static void pn7160_record_nci(uint8_t direction, const uint8_t* data, int length) {
  static const bool enabled = []() {
    const char *value = getenv("PN7160_DIAG_NCI_TRACE");
    return PN7160_ENABLE_DIAG && value && strcmp(value, "1") == 0;
  }();
  if (!enabled || !data || length <= 0) return;
  uint64_t entry = direction | ((uint64_t)(length > 255 ? 255 : length) << 8);
  for (int i = 0; i < length && i < 6; ++i) entry |= (uint64_t)data[i] << (16 + 8*i);
  std::lock_guard<std::mutex> lock(trace_mutex);
  trace_entries[trace_next % 256] = entry;
  ++trace_next;
  if (trace_used < 256) ++trace_used;
}
extern "C" int pn7160_diag_available(void) { return PN7160_ENABLE_DIAG; }
extern "C" uint32_t safe_nfc_nci_trace_count(void) {
  std::lock_guard<std::mutex> lock(trace_mutex); return trace_next;
}
extern "C" uint64_t safe_nfc_nci_trace_entry(uint32_t index) {
  std::lock_guard<std::mutex> lock(trace_mutex);
  uint32_t age = trace_next - index;
  return age && age <= trace_used ? trace_entries[index % 256] : 0;
}
extern "C" uint32_t pn7160_nci_trace_snapshot(uint64_t *out, uint32_t capacity, uint32_t *first) {
  if (!out || !first || !capacity) return 0;
  std::lock_guard<std::mutex> lock(trace_mutex);
  uint32_t count = capacity < trace_used ? capacity : trace_used;
  *first = trace_next - count;
  for (uint32_t i=0; i<count; ++i) out[i] = trace_entries[(*first+i)%256];
  return count;
}

/*******************************************************************************
**
** Function         OpenAndConfigure
**
** Description      Open and configure NFCC device
**
** Parameters       pConfig     - hardware information
**                  pLinkHandle - device handle
**
** Returns          NFC status:
**                  NFCSTATUS_SUCCESS - open_and_configure operation success
**                  NFCSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
NFCSTATUS NfccAltI2cTransport::OpenAndConfigure(pphTmlNfc_Config_t pConfig,
                                                void** pLinkHandle) {
  NXPLOG_TML_D("%s Enter", __func__);
  NXPLOG_TML_D("phTmlNfc_i2c_open_and_configure Alternative NFC\n");
  NXPLOG_TML_D("NFC - Assign IO pins\n");
  int status_value = -1;
  int Fd = -1;
  // Assign IO pins
  status_value = ConfigurePin();
  if(status_value != NFCSTATUS_SUCCESS)
    return NFCSTATUS_INVALID_DEVICE;
  const char* i2c_bus = pn7160_board_i2c_bus();
  const uint32_t i2c_address = pn7160_board_i2c_address();
  NXPLOG_TML_D("NFCHW - open I2C bus - %s\n", i2c_bus);

  // I2C bus
  Fd = open(i2c_bus, O_RDWR | O_NOCTTY);
  if (Fd < 0) {
    NXPLOG_TML_E("Could not open I2C bus '%s' (%s)", i2c_bus, strerror(errno));
    Close(NULL);
    return (NFCSTATUS_INVALID_DEVICE);
  }
  if (Fd == 0) {
    const int replacement = fcntl(Fd, F_DUPFD_CLOEXEC, 3);
    close(Fd);
    Fd = replacement;
    if (Fd < 0) { Close(NULL); return NFCSTATUS_INVALID_DEVICE; }
  }
  *pLinkHandle = (void*)((intptr_t)Fd);
  NXPLOG_TML_D("NFC - open I2C device - 0x%02x\n", i2c_address);

  // I2C slave address
  if (ioctl(Fd, I2C_SLAVE, i2c_address) < 0) {
    NXPLOG_TML_E("Cannot select I2C address (%s)\n", strerror(errno));
    Close((void*)((intptr_t)Fd));
    *pLinkHandle = NULL;
    return (NFCSTATUS_INVALID_DEVICE);
  }

  NfccReset((void*)((intptr_t)Fd), MODE_POWER_OFF);
  NfccReset((void*)((intptr_t)Fd), MODE_POWER_ON);

  NXPLOG_TML_D("%s exit", __func__);
  return NFCSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         Read
**
** Description      Reads requested number of bytes from NFCC device into given
**                  buffer
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToRead   - number of bytes requested to be read
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int NfccAltI2cTransport::Read(void* handle, uint8_t* buffer, int capacity) {
  if (!handle || !buffer || capacity <= 0) { errno = EINVAL; return -1; }
  const bool firmware = bFwDnldFlag;
  const int header = firmware ? FW_DNLD_HEADER_LEN : NORMAL_MODE_HEADER_LEN;
  if (capacity < header) { errno = EMSGSIZE; return -1; }
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  const uint32_t generation=read_generation.load();
  int received = 0;
  auto read_exact = [&](int target) -> bool {
    while (received < target) {
      int remaining = (int)std::chrono::duration_cast<std::chrono::milliseconds>(deadline-std::chrono::steady_clock::now()).count();
      if (remaining <= 0) { errno = ETIMEDOUT; return false; }
      if (wait4interrupt(remaining, generation) < 0) return false;
      ssize_t n = read((intptr_t)handle, buffer+received, target-received);
      if (n > 0) received += n;
      else if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
      else { if (!n) errno = EIO; return false; }
    }
    return true;
  };
  if (!read_exact(header)) return -1;
  const int total = header + buffer[firmware ? FW_DNLD_LEN_OFFSET : NORMAL_MODE_LEN_OFFSET] + (firmware ? CRC_LEN : 0);
  if (total > capacity) { errno = EMSGSIZE; return -1; }
  if (!read_exact(total)) return -1;
  pn7160_record_nci(1, buffer, received);
  return received;
}

/*******************************************************************************
**
** Function         Write
**
** Description      Writes requested number of bytes from given buffer into
**                  NFCC device
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToWrite  - number of bytes requested to be written
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*******************************************************************************/
int NfccAltI2cTransport::Write(void* pDevHandle, uint8_t* pBuffer,
                               int nNbBytesToWrite) {
  NXPLOG_TML_D("%s Enter", __func__);
  int ret;
  int numWrote = 0;
  int numBytes = nNbBytesToWrite;
  if (NULL == pDevHandle) {
    return -1;
  }
  if (fragmentation_enabled == I2C_FRAGMENATATION_DISABLED &&
      nNbBytesToWrite > FRAGMENTSIZE_MAX) {
    NXPLOG_TML_E(
        "i2c_write() data larger than maximum I2C  size,enable I2C "
        "fragmentation");
    return -1;
  }
  while (numWrote < nNbBytesToWrite) {
    if (fragmentation_enabled == I2C_FRAGMENTATION_ENABLED &&
        nNbBytesToWrite > FRAGMENTSIZE_MAX) {
      if (nNbBytesToWrite - numWrote > FRAGMENTSIZE_MAX) {
        numBytes = numWrote + FRAGMENTSIZE_MAX;
      } else {
        numBytes = nNbBytesToWrite;
      }
    }
    ret = write((intptr_t)pDevHandle, pBuffer + numWrote, numBytes - numWrote);
    if (ret > 0) {
      numWrote += ret;
      if (fragmentation_enabled == I2C_FRAGMENTATION_ENABLED &&
          numWrote < nNbBytesToWrite) {
        usleep(500);
      }
    } else if (ret == 0) {
      NXPLOG_TML_E("_i2c_write() EOF");
      return -1;
    } else {
      NXPLOG_TML_E("_i2c_write() errno : %x", errno);
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      }
      pn7160_record_nci(3, pBuffer, nNbBytesToWrite);
      return -1;
    }
  }
  pn7160_record_nci(2, pBuffer, numWrote);
  NXPLOG_TML_D("%s exit", __func__);
  return numWrote;
}

/*******************************************************************************
**
** Function         Close
**
** Description      Closes NFCC device
**
** Parameters       pDevHandle - device handle
**
** Returns          None
**
*******************************************************************************/
void NfccAltI2cTransport::Close(void* pDevHandle) {
  NXPLOG_TML_D("%s Enter", __func__);
  if (NULL != pDevHandle) {
    close((intptr_t)pDevHandle);
  }
  if (iEnableFd >= 0) close(iEnableFd);
  if (iInterruptFd >= 0) close(iInterruptFd);
  if (iFwDnldFd >= 0) close(iFwDnldFd);
  iEnableFd = iInterruptFd = iFwDnldFd = -1;
  NXPLOG_TML_D("%s exit", __func__);
  return;
}
