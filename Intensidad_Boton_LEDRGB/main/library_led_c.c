#include "library_led_c.h"
#include <math.h>

/* =====================================================
   CONFIGURE RGB PWM
   ===================================================== */
void config_led_rgb(led_rgb_t *led_rgb)
{
    // Configure PWM timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = led_rgb->speed_mode,
        .duty_resolution  = led_rgb->duty_resolution,
        .timer_num        = led_rgb->timer,
        .freq_hz          = led_rgb->frequency,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    /* =====================================================
       RED CHANNEL
       ===================================================== */
    ledc_channel_config_t ledc_channel_red = {
        .speed_mode = led_rgb->speed_mode,
        .channel    = led_rgb->led_red.channel,
        .timer_sel  = led_rgb->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = led_rgb->led_red.gpio_num,
        .duty       = led_rgb->led_red.duty,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_red));

    /* =====================================================
       GREEN CHANNEL
       ===================================================== */
    ledc_channel_config_t ledc_channel_green = {
        .speed_mode = led_rgb->speed_mode,
        .channel    = led_rgb->led_green.channel,
        .timer_sel  = led_rgb->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = led_rgb->led_green.gpio_num,
        .duty       = led_rgb->led_green.duty,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_green));

    /* =====================================================
       BLUE CHANNEL
       ===================================================== */
    ledc_channel_config_t ledc_channel_blue = {
        .speed_mode = led_rgb->speed_mode,
        .channel    = led_rgb->led_blue.channel,
        .timer_sel  = led_rgb->timer,
        .intr_type  = LEDC_INTR_DISABLE,
        .gpio_num   = led_rgb->led_blue.gpio_num,
        .duty       = led_rgb->led_blue.duty,
        .hpoint     = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_blue));
}

/* =====================================================
   CONFIGURE BUTTONS
   Includes:
   - RED
   - GREEN
   - BLUE
   - RESET
   ===================================================== */
void config_buttons_rgb(button_rgb_t *button_rgb)
{
    gpio_config_t button_config = {

        // GPIO mask for all buttons
        .pin_bit_mask =
            (1ULL << button_rgb->button_red.gpio_num)   |
            (1ULL << button_rgb->button_green.gpio_num) |
            (1ULL << button_rgb->button_blue.gpio_num)  |
            (1ULL << button_rgb->button_reset.gpio_num),

        // Configure as inputs
        .mode = GPIO_MODE_INPUT,

        // Enable internal pull-up resistors
        .pull_up_en = GPIO_PULLUP_ENABLE,

        // Disable pull-down resistors
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        // No interrupts
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&button_config));
}

/* =====================================================
   APPLY DUTY VALUES STORED INSIDE STRUCTURE
   ===================================================== */
void set_led_rgb_given_struct(led_rgb_t *led_rgb)
{
    // RED
    ESP_ERROR_CHECK(
        ledc_set_duty(
            led_rgb->speed_mode,
            led_rgb->led_red.channel,
            led_rgb->led_red.duty));

    // GREEN
    ESP_ERROR_CHECK(
        ledc_set_duty(
            led_rgb->speed_mode,
            led_rgb->led_green.channel,
            led_rgb->led_green.duty));

    // BLUE
    ESP_ERROR_CHECK(
        ledc_set_duty(
            led_rgb->speed_mode,
            led_rgb->led_blue.channel,
            led_rgb->led_blue.duty));

    // Update outputs
    ESP_ERROR_CHECK(
        ledc_update_duty(
            led_rgb->speed_mode,
            led_rgb->led_red.channel));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            led_rgb->speed_mode,
            led_rgb->led_green.channel));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            led_rgb->speed_mode,
            led_rgb->led_blue.channel));
}

/* =====================================================
   SET RGB USING RAW DUTY VALUES
   ===================================================== */
void set_led_rgb_given_values(
    led_rgb_t *led_rgb,
    uint32_t duty_red,
    uint32_t duty_green,
    uint32_t duty_blue)
{
    // Save values in structure
    led_rgb->led_red.duty   = duty_red;
    led_rgb->led_green.duty = duty_green;
    led_rgb->led_blue.duty  = duty_blue;

    // RED
    ESP_ERROR_CHECK(
        ledc_set_duty(
            led_rgb->speed_mode,
            led_rgb->led_red.channel,
            duty_red));

    // GREEN
    ESP_ERROR_CHECK(
        ledc_set_duty(
            led_rgb->speed_mode,
            led_rgb->led_green.channel,
            duty_green));

    // BLUE
    ESP_ERROR_CHECK(
        ledc_set_duty(
            led_rgb->speed_mode,
            led_rgb->led_blue.channel,
            duty_blue));

    // Update outputs
    ESP_ERROR_CHECK(
        ledc_update_duty(
            led_rgb->speed_mode,
            led_rgb->led_red.channel));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            led_rgb->speed_mode,
            led_rgb->led_green.channel));

    ESP_ERROR_CHECK(
        ledc_update_duty(
            led_rgb->speed_mode,
            led_rgb->led_blue.channel));
}

/* =====================================================
   SET RGB USING PERCENTAGES
   Example:
   100 = full brightness
   0   = off
   ===================================================== */
void set_led_rgb_percentage_given_values(
    led_rgb_t *led_rgb,
    int percentage_red,
    int percentage_green,
    int percentage_blue)
{
    // Calculate max PWM duty
    // Example:
    // 13 bits -> 8191
    uint32_t max_duty =
        (1 << led_rgb->duty_resolution) - 1;

    /* =====================================================
       COMMON ANODE RGB

       LOW  = ON
       HIGH = OFF

       PWM must be inverted
       ===================================================== */

    uint32_t duty_red =
        max_duty -
        (max_duty * percentage_red / 100);

    uint32_t duty_green =
        max_duty -
        (max_duty * percentage_green / 100);

    uint32_t duty_blue =
        max_duty -
        (max_duty * percentage_blue / 100);

    // Apply new values
    set_led_rgb_given_values(
        led_rgb,
        duty_red,
        duty_green,
        duty_blue);
}