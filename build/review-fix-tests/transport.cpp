#include <chrono>
#include <atomic>
#include <thread>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <cstdio>
#define NXPLOG_TML_D(...)
#define NXPLOG_TML_E(...)
#define FALSE false
#define TRUE true
#define NORMAL_MODE_HEADER_LEN 3
#define FW_DNLD_HEADER_LEN 2
#define NORMAL_MODE_LEN_OFFSET 2
#define FW_DNLD_LEN_OFFSET 1
#define CRC_LEN 2
bool irq_high=true, writes_fail=false, interrupt_read=false, edge_event=false;
std::vector<unsigned char> input;
size_t position=0,chunk=1;
int mock_lseek(int,int,int){return 0;}
ssize_t mock_read(int fd,void* out,size_t n){
 if(fd==0){memcpy(out,irq_high?"1\n":"0\n",2);return 2;}
 if(interrupt_read){interrupt_read=false;errno=EINTR;return -1;}
 if(position==input.size())return 0;
 size_t take=std::min(n,std::min(chunk,input.size()-position));
 memcpy(out,input.data()+position,take);position+=take;return take;}
ssize_t mock_write(int,const void*,size_t n){if(writes_fail){errno=EACCES;return -1;}return n;}
int mock_poll(pollfd* fd,int,int){if(edge_event){irq_high=true;edge_event=false;fd->revents=POLLPRI|POLLERR;return 1;}std::this_thread::sleep_for(std::chrono::milliseconds(2));return 0;}
#define lseek mock_lseek
#define read mock_read
#define write mock_write
#define poll mock_poll
void pn7160_board_reset_wait(){}
void pn7160_record_nci(int,const unsigned char*,int){}
class NfccAltTransport {public:
 int iInterruptFd=0,iEnableFd=8,iFwDnldFd=-1;
 std::atomic<uint32_t> read_generation{0};bool bFwDnldFlag=false;
 int GetIrqState(void*);int wait4interrupt(int,uint32_t);
 int gpio_set_ven(int);int gpio_set_fwdl(int);
};
class NfccAltI2cTransport:public NfccAltTransport {public:int Read(void*,unsigned char*,int);};
class NfccAltSpiTransport:public NfccAltTransport {public:int Read(void*,unsigned char*,int);};
int SpiRead(int fd,unsigned char* p,int n){return mock_read(fd,p,n);}
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
int NfccAltSpiTransport::Read(void* handle, uint8_t* buffer, int capacity) {
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
      ssize_t n = SpiRead((intptr_t)handle, buffer+received, target-received);
      if (n > 0) received += n;
      else if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
      else { if (!n) errno = EIO; return false; }
    }
    return true;
  };
  if (!read_exact(header)) return -1;
  if (buffer[0] == 0xFF && buffer[1] == 0xFF) { errno=EIO; return -1; }
  const int total = header + buffer[firmware ? FW_DNLD_LEN_OFFSET : NORMAL_MODE_LEN_OFFSET] + (firmware ? CRC_LEN : 0);
  if (total > capacity) { errno = EMSGSIZE; return -1; }
  if (!read_exact(total)) return -1;
  return received;
}
template<class T> void receive_tests(){T t;unsigned char b[8]={};
 input={0x60,0,2,0xAA,0xBB};position=0;interrupt_read=true;assert(t.Read((void*)99,b,8)==5);assert(!memcmp(b,input.data(),5));
 position=0;memset(b,0xEF,8);assert(t.Read((void*)99,b,4)==-1&&errno==EMSGSIZE);assert(b[4]==0xEF&&position==3);
 input={0x60,0,0};position=0;assert(t.Read((void*)99,b,8)==3);
 input={0x60,0,2,0xAA};position=0;assert(t.Read((void*)99,b,8)==-1&&errno==EIO);
 assert(t.Read((void*)99,b,1)==-1&&errno==EMSGSIZE);assert(t.Read(nullptr,b,8)==-1&&errno==EINVAL);
}
int main(){NfccAltTransport t;assert(t.GetIrqState(nullptr)==1);
 irq_high=false;assert(t.wait4interrupt(12,0)==-1&&errno==ETIMEDOUT);
 std::thread cancel([&](){std::this_thread::sleep_for(std::chrono::milliseconds(10));++t.read_generation;});
 assert(t.wait4interrupt(1000,0)==-1&&errno==ECANCELED);cancel.join();
 edge_event=true;assert(t.wait4interrupt(100,1)==0); // sysfs POLLPRI|POLLERR is normal
 writes_fail=true;assert(t.gpio_set_ven(1)==-1&&errno==EACCES);writes_fail=false;
 assert(t.gpio_set_fwdl(0)==0);assert(t.gpio_set_fwdl(1)==-1&&errno==ENOTSUP);
 receive_tests<NfccAltI2cTransport>();receive_tests<NfccAltSpiTransport>();
 puts("PASS: fd0, timeout, cancellation, sysfs edges, GPIO errors, I2C/SPI short frames/EINTR/EOF/capacity/zero payload");}
