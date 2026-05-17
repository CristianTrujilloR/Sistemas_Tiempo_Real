#include "library_led_c.h"

void config_led_rgb(led_rgb_t *led_rgb)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = led_rgb->speed_mode,
        .timer_num        = led_rgb->timer,
        .duty_resolution  = led_rgb->duty_resoltion,
        .freq_hz          = led_rgb->frecuency,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration for each LED
    led_t leds[] = {led_rgb->led_red, led_rgb->led_green, led_rgb->led_blue};
    for (int i = 0; i < 3; i++) {
        ledc_channel_config_t ledc_channel = {
            .speed_mode     = led_rgb->speed_mode,
            .channel        = leds[i].channel,
            .timer_sel      = led_rgb->timer,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = leds[i].gpio_num,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    }
} 