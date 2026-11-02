#ifndef PN7160_DIAG_H
#define PN7160_DIAG_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Set PN7160_DIAG_NCI_TRACE=1 before the first exchange; no runtime writes.
 * Entry: bits 0..7 direction (1=RX,2=TX,3=TX failure), 8..15 length capped
 * at 255; 16..63 first six bytes, lowest byte first. Capacity is 256 entries.
 * Legacy entry reads return 0 for overwritten/unpublished indices.
 * snapshot copies a consistent window and reports its first sequence index.
 * Counter is uint32 and wraps; snapshot is preferred over count/entry pairs.
 */
int pn7160_diag_available(void);
uint32_t safe_nfc_nci_trace_count(void);
uint64_t safe_nfc_nci_trace_entry(uint32_t index);
uint32_t pn7160_nci_trace_snapshot(uint64_t *entries, uint32_t capacity, uint32_t *first);
/* Historical SDK packaging capability marker, retained for compatibility. */
int safe_nfc_portable_runtime_build(void);
#ifdef __cplusplus
}
#endif
#endif
