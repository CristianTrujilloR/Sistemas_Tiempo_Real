#ifndef CURTAIN_MANAGER_H_
#define CURTAIN_MANAGER_H_

#include <stdint.h>

void curtain_init(void);

void curtain_set_mode(uint8_t mode);

void curtain_set_manual_percent(uint8_t percent);

void curtain_set_auto_percent(uint8_t percent);

uint8_t curtain_get_mode(void);

uint8_t curtain_get_percent(void);

#endif