#define PN7160_INTERNAL
#include "pn7160_board_config.h"
#include "pn7160_board_defaults.h"
#include <string.h>
int main(void) {
    pn7160_config_t c;
    if (pn7160_board_begin() != PN7160_CONFIG_MISSING) return 1;
    if (pn7160_get_default_config(0, sizeof(c)) != PN7160_CONFIG_INVALID) return 2;
    if (pn7160_get_default_config(&c, sizeof(c)-1) != PN7160_CONFIG_INVALID) return 3;
    if (pn7160_get_default_config(&c, sizeof(c))) return 4;
    if (c.abi_version != 1 || c.struct_size != sizeof(c)) return 5;
    if (strcmp(c.i2c_device, PN7160_DEFAULT_I2C_DEVICE) ||
        c.i2c_address != PN7160_DEFAULT_I2C_ADDRESS || c.irq_gpio != PN7160_DEFAULT_IRQ_GPIO ||
        c.ven_gpio != PN7160_DEFAULT_VEN_GPIO || c.dwl_req_gpio != PN7160_DEFAULT_DWL_REQ_GPIO ||
        c.reset_delay_us != PN7160_DEFAULT_RESET_DELAY_US) return 6;
    if (pn7160_board_begin() != PN7160_CONFIG_MISSING) return 7;
    if (pn7160_set_config(&c) || pn7160_board_begin()) return 8;
    if (pn7160_set_config(&c) != PN7160_CONFIG_BUSY) return 9;
    pn7160_board_end();
    c.i2c_address = 0x80;
    if (pn7160_set_config(&c) != PN7160_CONFIG_INVALID) return 10;
    return 0;
}
