#include <chrono>
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
#include <NfccI2cTransport.h>
#include <NfccAltTransport.h>
#include <phNfcStatus.h>
#include <phNxpLog.h>
#include <string.h>
#include "phNxpNciHal_utils.h"

#define CRC_LEN 2
#define NORMAL_MODE_HEADER_LEN 3
#define FW_DNLD_HEADER_LEN 2
#define FW_DNLD_LEN_OFFSET 1
#define NORMAL_MODE_LEN_OFFSET 2
#define FRAGMENTSIZE_MAX PHNFC_I2C_FRAGMENT_SIZE
extern phTmlNfc_i2cfragmentation_t fragmentation_enabled;
extern phTmlNfc_Context_t* gpphTmlNfc_Context;

NfccAltTransport::NfccAltTransport() {
  iEnableFd = iInterruptFd = iFwDnldFd = -1;
}

/*******************************************************************************
**
** Function         Flushdata
**
** Description      Reads payload of FW rsp from NFCC device into given buffer
**
** Parameters       pDevHandle - valid device handle
**                  pBuffer    - buffer for read data
**                  numRead    - number of bytes read by calling function
**
** Returns          always returns -1
**
*******************************************************************************/
int NfccAltTransport::Flushdata(void* pDevHandle, uint8_t* pBuffer,
                                int numRead) {
  int retRead = 0;
  uint16_t totalBtyesToRead =
      pBuffer[FW_DNLD_LEN_OFFSET] + FW_DNLD_HEADER_LEN + CRC_LEN;
  /* we shall read totalBtyesToRead-1 as one byte is already read by calling
   * function*/
  retRead = read((intptr_t)pDevHandle, pBuffer + numRead, totalBtyesToRead - 1);
  if (retRead > 0) {
    numRead += retRead;
    phNxpNciHal_print_packet("RECV", pBuffer, numRead);
  } else if (retRead == 0) {
    NXPLOG_TML_E("%s _i2c_read() [pyld] EOF", __func__);
  } else {
    if (bFwDnldFlag == false) {
      NXPLOG_TML_D("%s _i2c_read() [hdr] received", __func__);
      phNxpNciHal_print_packet("RECV", pBuffer - numRead,
                               NORMAL_MODE_HEADER_LEN);
    }
    NXPLOG_TML_E("%s _i2c_read() [pyld] errno : %x", __func__, errno);
  }
  SemPost();
  return -1;
}

/*******************************************************************************
**
** Function         Reset
**
** Description      Reset NFCC device, using VEN pin
**
** Parameters       pDevHandle     - valid device handle
**                  eType          - reset level
**
** Returns           0   - reset operation success
**                  -1   - reset operation failure
**
*******************************************************************************/
int NfccAltTransport::NfccReset(void* pDevHandle, NfccResetType eType) {
  int ret = 0;
  NXPLOG_TML_D("%s, VEN eType %ld", __func__, eType);

  if (NULL == pDevHandle) {
    return -1;
  }
  switch (eType) {
    case MODE_POWER_OFF:
      if (gpio_set_fwdl(0) < 0) return -1;
      if (gpio_set_ven(0) < 0) return -1;
      break;
    case MODE_POWER_ON:
      if (gpio_set_fwdl(0) < 0) return -1;
      if (gpio_set_ven(1) < 0) return -1;
      break;
    case MODE_FW_DWNLD_WITH_VEN:
      if (gpio_set_fwdl(1) < 0) return -1;
      if (gpio_set_ven(0) < 0) return -1;
      if (gpio_set_ven(1) < 0) return -1;
      break;
    case MODE_FW_DWND_HIGH:
      if (gpio_set_fwdl(1) < 0) return -1;
      break;
    case MODE_POWER_RESET:
      if (gpio_set_ven(0) < 0) return -1;
      if (gpio_set_ven(1) < 0) return -1;
      break;
    case MODE_FW_GPIO_LOW:
      if (gpio_set_fwdl(0) < 0) return -1;
      break;
    default:
      NXPLOG_TML_E("%s, VEN eType %ld", __func__, eType);
      return -1;
  }
  if ((eType != MODE_FW_DWNLD_WITH_VEN) && (eType != MODE_FW_DWND_HIGH)) {
    EnableFwDnldMode(false);
  }
  if ((eType == MODE_FW_DWNLD_WITH_VEN) || (eType == MODE_FW_DWND_HIGH)) {
    EnableFwDnldMode(true);
  }

  return ret;
}

/*******************************************************************************
**
** Function         GetNfcState
**
** Description      Get NFC state
**
** Parameters       pDevHandle     - valid device handle
** Returns           0   - unknown
**                   1   - FW DWL
**                   2 	 - NCI
**
*******************************************************************************/
int NfccAltTransport::GetNfcState(void* pDevHandle) {
  int ret = NFC_STATE_UNKNOWN;
  NXPLOG_TML_D("%s ", __func__);
  if (NULL == pDevHandle) {
    return ret;
  }
  ret = ioctl((intptr_t)pDevHandle, NFC_GET_NFC_STATE);
  NXPLOG_TML_D("%s :nfc state = %d", __func__, ret);
  return ret;
}
/*******************************************************************************
**
** Function         EnableFwDnldMode
**
** Description      updates the state to Download mode
**
** Parameters       True/False
**
** Returns          None
*******************************************************************************/
void NfccAltTransport::EnableFwDnldMode(bool mode) { bFwDnldFlag = mode; }

/*******************************************************************************
**
** Function         IsFwDnldModeEnabled
**
** Description      Returns the current mode
**
** Parameters       none
**
** Returns           Current mode download/NCI
*******************************************************************************/
bool_t NfccAltTransport::IsFwDnldModeEnabled(void) { return bFwDnldFlag; }

/*******************************************************************************
**
** Function         SemPost
**
** Description      sem_post 2c_read / write
**
** Parameters       none
**
** Returns          none
*******************************************************************************/
void NfccAltTransport::SemPost() {
  int sem_val = 0;
  sem_getvalue(&mTxRxSemaphore, &sem_val);
  if (sem_val == 0) {
    sem_post(&mTxRxSemaphore);
  }
}

/*******************************************************************************
**
** Function         SemTimedWait
**
** Description      Timed sem_wait for avoiding i2c_read & write overlap
**
** Parameters       none
**
** Returns          Sem_wait return status
*******************************************************************************/
int NfccAltTransport::SemTimedWait() {
  NFCSTATUS status = NFCSTATUS_FAILED;
  long sem_timedout = 500 * 1000 * 1000;
  int s = 0;
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 0;
  ts.tv_nsec += sem_timedout;
  while ((s = sem_timedwait(&mTxRxSemaphore, &ts)) == -1 && errno == EINTR) {
    continue; /* Restart if interrupted by handler */
  }
  if (s != -1) {
    status = NFCSTATUS_SUCCESS;
  } else if (errno == ETIMEDOUT && s == -1) {
    NXPLOG_TML_E("%s :timed out errno = 0x%x", __func__, errno);
  }
  return status;
}

/*******************************************************************************
**
** Function         GetIrqState
**
** Description      Get state of IRQ GPIO
**
** Parameters       pDevHandle - valid device handle
**
** Returns          The state of IRQ line i.e. +ve if read is pending else Zer0.
**                  In the case of IOCTL error, it returns -ve value.
**
*******************************************************************************/
int NfccAltTransport::GetIrqState(void* pDevHandle) {
  int ret = -1;

  NXPLOG_TML_D("%s Enter", __func__);
  int len;
  char buf[2];

  if (iInterruptFd < 0) {
    NXPLOG_TML_E("Error with interrupt-detect pin (%d)", iInterruptFd);
    return (-1);
  }

  // Seek to the start of the file
  if (lseek(iInterruptFd, 0, SEEK_SET) < 0) return -1;

  // Read the field_detect line
  len = read(iInterruptFd, buf, 2);

  if (len != 2) {
    NXPLOG_TML_E("Error with interrupt-detect pin (%s)", strerror(errno));
    errno=EIO; return (-1);
  }

  NXPLOG_TML_D("%s exit: state = %d", __func__, (buf[0] != '0'));
  return (buf[0] != '0');
}

int NfccAltTransport::verifyPin(int pin, int isoutput, int edge) {
  char buf[40];
  // Check if gpio pin has already been created
  int hasGpio = 0;
  NXPLOG_TML_D("%s Enter", __func__);
  sprintf(buf, "/sys/class/gpio/gpio%d", pin);
  NXPLOG_TML_D("Pin %s\n", buf);
  int fd = open(buf, O_RDONLY);
  if (fd < 0) {
    // Pin not exported yet
    NXPLOG_TML_D("Create pin %s\n", buf);
    if ((fd = open("/sys/class/gpio/export", O_WRONLY)) >= 0) {
      sprintf(buf, "%d", pin);
      if (write(fd, buf, strlen(buf)) == strlen(buf)) {
        hasGpio = 1;
        usleep(100 * 1000);
      }
    } else {
      NXPLOG_TML_E("open failed for /sys/class/gpio/export\n");
      return -1;
    }
  } else {
    NXPLOG_TML_E("System already has pin %s\n", buf);
    hasGpio = 1;
  }
  close(fd);

  if (hasGpio) {
    // Make sure it is an output
    sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin);
    NXPLOG_TML_D("Direction %s\n", buf);
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
      NXPLOG_TML_E("Could not open direction port '%s' (%s)", buf,
                   strerror(errno));
      return -1;
    } else {
      if (isoutput) {
        if (write(fd, "out", 3) != 3) { int saved=errno; close(fd); errno=saved?saved:EIO; return -1; }
          {
          NXPLOG_TML_D("Pin %d now an output\n", pin);
        }
        close(fd);

        // Open pin and make sure it is off
        sprintf(buf, "/sys/class/gpio/gpio%d/value", pin);
        fd = open(buf, O_RDWR);
        if (fd < 0) {
          NXPLOG_TML_E("Could not open value port '%s' (%s)", buf,
                       strerror(errno));
          return -1;
        } else {
          if (write(fd, "0", 1) != 1) { int saved=errno; close(fd); errno=saved?saved:EIO; return -1; }
          {
            NXPLOG_TML_D("Pin %d now off\n", pin);
          }
          return (fd);  // Success
        }
      } else {
        if (write(fd, "in", 2) != 2) { int saved=errno; close(fd); errno=saved?saved:EIO; return -1; }
          {
          NXPLOG_TML_D("Pin %d now an input\n", pin);
        }
        close(fd);

        if (edge != EDGE_NONE) {
          // Open pin edge control
          sprintf(buf, "/sys/class/gpio/gpio%d/edge", pin);
          NXPLOG_TML_D("Edge %s\n", buf);
          fd = open(buf, O_RDWR);
          if (fd < 0) {
            NXPLOG_TML_E("Could not open edge port '%s' (%s)", buf,
                         strerror(errno));
            return -1;
          } else {
            char* edge_str = "none";
            switch (edge) {
              case EDGE_RISING:
                edge_str = "rising";
                break;
              case EDGE_FALLING:
                edge_str = "falling";
                break;
              case EDGE_BOTH:
                edge_str = "both";
                break;
            }
            int l = strlen(edge_str);
            NXPLOG_TML_D("Edge-string %s - %d\n", edge_str, l);
            if (write(fd, edge_str, l) != l) { int saved=errno; close(fd); errno=saved?saved:EIO; return -1; }
          {
              NXPLOG_TML_D("Pin %d trigger on %s\n", pin, edge_str);
            }
            close(fd);
          }
        }

        // Open pin
        sprintf(buf, "/sys/class/gpio/gpio%d/value", pin);
        NXPLOG_TML_D("Value %s\n", buf);
        fd = open(buf, O_RDONLY);
        if (fd < 0) {
          NXPLOG_TML_E("Could not open value port '%s' (%s)", buf,
                       strerror(errno));
          return -1;
        } else {
          return (fd);  // Success
        }
      }
    }
  }
  return (-1);
}
int NfccAltTransport::gpio_set_ven(int value) {
  if (iEnableFd < 0) { errno=EBADF; return -1; }
  if (write(iEnableFd, value ? "1" : "0", 1) != 1) return -1;
  pn7160_board_reset_wait(); return 0;
}
int NfccAltTransport::gpio_set_fwdl(int value) {
  if (iFwDnldFd < 0) { if (!value) return 0; errno=ENOTSUP; return -1; }
  if (write(iFwDnldFd, value ? "1" : "0", 1) != 1) return -1;
  pn7160_board_reset_wait(); return 0;
}
int NfccAltTransport::wait4interrupt(int timeout_ms, uint32_t generation) {
  auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(timeout_ms);
  while (true) {
    if (read_generation.load() != generation) { errno=ECANCELED; return -1; }
    int state=GetIrqState(NULL);
    if (state < 0) return -1;
    if (state > 0) return 0;
    int remaining=(int)std::chrono::duration_cast<std::chrono::milliseconds>(deadline-std::chrono::steady_clock::now()).count();
    if (remaining <= 0) { errno=ETIMEDOUT; return -1; }
    struct pollfd fd={iInterruptFd, POLLPRI, 0};
    int result=poll(&fd,1,remaining < 50 ? remaining : 50);
    if (result < 0 && errno==EINTR) continue;
    if (result < 0) return -1;
    if (!result) continue;
    if ((fd.revents & (POLLHUP|POLLNVAL)) || ((fd.revents & POLLERR) && !(fd.revents & POLLPRI))) { errno=EIO; return -1; }
  }
}

/*****************************************************************************
   **
   ** Function         ConfigurePin
   **
   ** Description      Configure Pins such as IRQ, VEN, Firmware Download
   **
   ** Parameters       none
   **
   ** Returns           NFCSTATUS_SUCCESS - on Success/ -1 on Failure
   ****************************************************************************/
int NfccAltTransport::ConfigurePin()
{
  // Assign IO pins
  if (!pn7160_board_configured()) return NFCSTATUS_INVALID_DEVICE;
  const int irq = pn7160_board_irq(), ven = pn7160_board_ven(), dwl = pn7160_board_dwl();
  iInterruptFd = verifyPin(irq, 0, EDGE_RISING);
  if (iInterruptFd < 0) return NFCSTATUS_INVALID_DEVICE;
  iEnableFd = verifyPin(ven, 1, EDGE_NONE);
  if (iEnableFd < 0) {
    close(iInterruptFd); iInterruptFd = -1;
    return NFCSTATUS_INVALID_DEVICE;
  }
  iFwDnldFd = dwl < 0 ? -1 : verifyPin(dwl, 1, EDGE_NONE);
  if (dwl >= 0 && iFwDnldFd < 0) {
    close(iInterruptFd); close(iEnableFd);
    iInterruptFd = iEnableFd = -1;
    return NFCSTATUS_INVALID_DEVICE;
  }
  return NFCSTATUS_SUCCESS;
}
