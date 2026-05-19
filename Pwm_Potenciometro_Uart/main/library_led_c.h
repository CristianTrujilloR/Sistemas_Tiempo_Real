#ifndef LIBRARY_LED_C_H
#define LIBRARY_LED_C_H

#include "driver/ledc.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <stdint.h>

/* =====================================================
   ESTRUCTURA LED

   Guarda:
   - duty PWM actual
   - GPIO asociado
   - canal PWM LEDC
   ===================================================== */
typedef struct
{
    uint32_t duty;
    gpio_num_t gpio_num;
    ledc_channel_t channel;

} led_t;

/* =====================================================
   ESTRUCTURA RGB

   Contiene:
   - LED rojo
   - LED verde
   - LED azul
   - configuración general PWM
   ===================================================== */
typedef struct
{
    led_t led_red;
    led_t led_green;
    led_t led_blue;

    ledc_timer_t timer;

    ledc_timer_bit_t duty_resolution;

    uint32_t frequency;

    ledc_mode_t speed_mode;

} led_rgb_t;

/* =====================================================
   ESTRUCTURA BOTÓN
   ===================================================== */
typedef struct
{
    gpio_num_t gpio_num;

} button_t;

/* =====================================================
   ESTRUCTURA BOTONES RGB
   ===================================================== */
typedef struct
{
    button_t button_aux;

} button_rgb_t;

/* =====================================================
   PROTOTIPOS DE FUNCIONES
   ===================================================== */

/* Configura PWM RGB */
void config_led_rgb(led_rgb_t *led_rgb);

/* Configura botones */
void config_buttons_rgb(button_rgb_t *button_rgb);

/* Envía duty PWM manual */
void set_led_rgb_given_values(
    led_rgb_t *led_rgb,
    uint32_t duty_red,
    uint32_t duty_green,
    uint32_t duty_blue);

/* Envía brillo usando porcentajes */
void set_led_rgb_percentage_given_values(
    led_rgb_t *led_rgb,
    int percentage_red,
    int percentage_green,
    int percentage_blue);

/* Apaga completamente el RGB */
void led_rgb_off(led_rgb_t *led_rgb);

/* Activa solo un color */
void led_rgb_set_single_color(
    led_rgb_t *led_rgb,
    uint32_t pwm,
    char color);

#endif