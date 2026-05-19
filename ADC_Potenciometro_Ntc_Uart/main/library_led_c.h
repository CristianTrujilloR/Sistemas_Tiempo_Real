#ifndef LIBRARY_LED_C_H
#define LIBRARY_LED_C_H

#include "driver/ledc.h"
#include "driver/gpio.h"

#include "esp_adc/adc_oneshot.h"

#include "esp_adc/adc_cali.h"

#include <stdio.h>

#include <stdint.h>

#include <math.h>

/* =====================================================
   PARÁMETROS NTC
   ===================================================== */

#define R_FIJA                 100000.0f

#define NTC_R_NOMINAL          100000.0f

#define NTC_TEMP_NOMINAL       298.15f

#define NTC_BETA               4190.0f

#define VOLTAJE_ENTRADA        3.3f

/* =====================================================
   ESTRUCTURA LED
   ===================================================== */

typedef struct
{
    uint32_t duty;

    gpio_num_t gpio_num;

    ledc_channel_t channel;

} led_t;

/* =====================================================
   RGB
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
   BOTÓN
   ===================================================== */

typedef struct
{
    gpio_num_t gpio_num;

} button_t;

/* =====================================================
   BOTONES RGB
   ===================================================== */

typedef struct
{
    button_t button_aux;

} button_rgb_t;

/* =====================================================
   RANGOS TEMPERATURA
   ===================================================== */

typedef struct
{
    int red_min;
    int red_max;

    int green_min;
    int green_max;

    int blue_min;
    int blue_max;

} temp_range_t;

/* =====================================================
   FUNCIONES RGB
   ===================================================== */

void config_led_rgb(led_rgb_t *led_rgb);

void config_buttons_rgb(button_rgb_t *button_rgb);

void set_led_rgb_given_values(
    led_rgb_t *led_rgb,
    uint32_t duty_red,
    uint32_t duty_green,
    uint32_t duty_blue);

void set_led_rgb_percentage_given_values(
    led_rgb_t *led_rgb,
    int percentage_red,
    int percentage_green,
    int percentage_blue);

void led_rgb_off(led_rgb_t *led_rgb);

void led_rgb_set_single_color(
    led_rgb_t *led_rgb,
    uint32_t pwm,
    char color);

/* =====================================================
   FUNCIONES ADC / NTC
   ===================================================== */

int read_adc_calibrated_mv(
    adc_oneshot_unit_handle_t adc_handle,
    adc_cali_handle_t adc_cali_handle,
    adc_channel_t channel);

float calculate_ntc_temperature(
    int voltage_mv);

/* =====================================================
   FUNCIONES TEMPERATURA
   ===================================================== */

char get_color_from_temperature(
    float temperature,
    temp_range_t *ranges);

void parse_temperature_command(
    char *data,
    temp_range_t *ranges);

#endif