#include "driver/ledc.h"
#include "driver/gpio.h"
#include <stdio.h>

/* =====================================================
   LED STRUCTURE
   Stores:
   - PWM duty
   - GPIO
   - PWM channel
   ===================================================== */
typedef struct
{
    uint32_t duty;
    gpio_num_t gpio_num;
    ledc_channel_t channel;

} led_t;

/* =====================================================
   RGB STRUCTURE
   Stores:
   - Red LED
   - Green LED
   - Blue LED
   - PWM configuration
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
   BUTTON STRUCTURE
   Stores button GPIO
   ===================================================== */
typedef struct
{
    gpio_num_t gpio_num;

} button_t;

/* =====================================================
   RGB BUTTONS STRUCTURE
   Includes:
   - RED button
   - GREEN button
   - BLUE button
   - RESET button
   ===================================================== */
typedef struct
{
    button_t button_red;
    button_t button_green;
    button_t button_blue;

    // New reset button
    button_t button_reset;

} button_rgb_t;

/* =====================================================
   FUNCTION DECLARATIONS
   ===================================================== */

// Configure RGB PWM
void config_led_rgb(led_rgb_t *led_rgb);

// Configure buttons
void config_buttons_rgb(button_rgb_t *button_rgb);

// Apply RGB values stored in structure
void set_led_rgb_given_struct(led_rgb_t *led_rgb);

// Set RGB using percentages
void set_led_rgb_percentage_given_values(
    led_rgb_t *led_rgb,
    int percentage_red,
    int percentage_green,
    int percentage_blue);

// Set RGB using raw duty values
void set_led_rgb_given_values(
    led_rgb_t *led_rgb,
    uint32_t duty_red,
    uint32_t duty_green,
    uint32_t duty_blue);