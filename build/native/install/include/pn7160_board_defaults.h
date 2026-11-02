#ifndef PN7160_BOARD_DEFAULTS_H
#define PN7160_BOARD_DEFAULTS_H
/* Optional generated board profile; macros can also be supplied by CPPFLAGS. */
#ifdef PN7160_USE_BUILD_DEFAULTS
#include "pn7160_board_build_config.h"
#endif
#ifndef PN7160_DEFAULT_I2C_DEVICE
#define PN7160_DEFAULT_I2C_DEVICE "/dev/i2c-1"
#endif
#ifndef PN7160_DEFAULT_I2C_ADDRESS
#define PN7160_DEFAULT_I2C_ADDRESS 0x28
#endif
#ifndef PN7160_DEFAULT_IRQ_GPIO
#define PN7160_DEFAULT_IRQ_GPIO 23
#endif
#ifndef PN7160_DEFAULT_VEN_GPIO
#define PN7160_DEFAULT_VEN_GPIO 24
#endif
#ifndef PN7160_DEFAULT_DWL_REQ_GPIO
#define PN7160_DEFAULT_DWL_REQ_GPIO (-1)
#endif
#ifndef PN7160_DEFAULT_RESET_DELAY_US
#define PN7160_DEFAULT_RESET_DELAY_US 10000
#endif
#endif
