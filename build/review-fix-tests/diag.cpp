#include <mutex>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include <cassert>
#include <cstdio>
#include "pn7160_diag.h"
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


int main(){setenv("PN7160_DIAG_NCI_TRACE","1",1);std::vector<std::thread> writers;
 for(int k=0;k<4;k++)writers.emplace_back([](){uint8_t data[2]={0x60,0};for(int n=0;n<5000;n++)pn7160_record_nci(2,data,2);});
 for(int n=0;n<1000;n++){uint64_t e[256];uint32_t first;auto count=pn7160_nci_trace_snapshot(e,256,&first);for(uint32_t j=0;j<count;j++)assert((e[j]&255)==2);}
 for(auto& t:writers)t.join();assert(safe_nfc_nci_trace_count()==20000);
 uint64_t e[256];uint32_t first;assert(pn7160_nci_trace_snapshot(e,256,&first)==256&&first==19744);
 assert(safe_nfc_nci_trace_entry(0)==0);puts("PASS: concurrent diagnostic publication/snapshot and overwritten indices");}
