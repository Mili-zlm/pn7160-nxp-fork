#ifndef PN7160_BOARD_INTERNAL_H
#define PN7160_BOARD_INTERNAL_H
#include "pn7160_board_config.h"
#ifdef __cplusplus
extern "C" {
#endif
const char *pn7160_board_config_directory(void);
int pn7160_board_begin(void);
void pn7160_board_end(void);
int pn7160_board_configured(void);
const char *pn7160_board_i2c_bus(void);
uint32_t pn7160_board_i2c_address(void);
int32_t pn7160_board_irq(void);
int32_t pn7160_board_ven(void);
int32_t pn7160_board_dwl(void);
void pn7160_board_reset_wait(void);
#ifdef __cplusplus
}
#endif
#endif
