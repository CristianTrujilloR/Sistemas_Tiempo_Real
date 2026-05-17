
#include <stdio.h>
#include "driver/ledc.h"

typedef struct{
    uint32_t duty;
    gpio_num_t gpio_num;
    ledc_channel_t channel;

}led_t;

typedef struct library_led_c
{
    led_t led_red;
    led_t led_green;
    led_t led_blue;
    ledc_timer_t timer;
    led_timer_bit_t duty_resoltion;
    uint32_t frecuency;
    ledc_mode_t speed_mode;
};led_rgb_t;
