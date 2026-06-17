#ifndef FAN_H
#define FAN_H

#include <stdint.h>

/* Inicialización completa (PWM + alarma + task) */
void fan_start(void);

/* Inicialización base (si se usa sin task) */
void fan_init(void);

/* Control manual del ventilador */
void fan_set_manual(uint8_t percent);

/* Cambio de modo AUTO / MANUAL */
void fan_set_mode(uint8_t mode);

/* Control automático por temperatura */
void fan_update_auto(float temp, float t_desired, float t_max);

/* Estado actual del modo */
uint8_t fan_get_mode(void);

#endif