#ifndef PN7160_BOARD_CONFIG_H
#define PN7160_BOARD_CONFIG_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define PN7160_CONFIG_ABI_VERSION 1u
enum { PN7160_CONFIG_OK=0, PN7160_CONFIG_INVALID=-1,
       PN7160_CONFIG_BUSY=-2, PN7160_CONFIG_ABI_MISMATCH=-3,
       PN7160_CONFIG_MISSING=-4 };
typedef struct pn7160_config {
    uint32_t abi_version;
    uint32_t struct_size;
    const char *i2c_device; /* copied by setter; absolute device path */
    uint32_t i2c_address;  /* unshifted 7-bit address */
    int32_t irq_gpio;     /* Linux sysfs global number; not physical pin */
    int32_t ven_gpio;
    int32_t dwl_req_gpio; /* -1 disables this optional output */
    uint32_t reset_delay_us;
    uint32_t reserved[4]; /* must be zero for ABI v1 */
} pn7160_config_t;
uint32_t pn7160_config_abi_version(void);
/* Returns library build defaults without changing active configuration.
 * i2c_device points to library-owned static storage; do not free it. */
int pn7160_get_default_config(pn7160_config_t *config, uint32_t config_size);
int pn7160_set_config(const pn7160_config_t *config);
/* Call before any NXP configuration read; directory must be absolute.
 * Configuration is cached: changing it after the first read returns BUSY. */
int pn7160_set_config_directory(const char *directory);

#ifdef __cplusplus
}
#endif
#endif
