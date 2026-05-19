#include "library_led_c.h"

/* =====================================================
   CONFIGURAR PWM RGB
   ===================================================== */
void config_led_rgb(led_rgb_t *led_rgb)
{
    /* =====================================================
       TIMER PWM
       ===================================================== */
    ledc_timer_config_t ledc_timer = {

        .speed_mode       = led_rgb->speed_mode,

        .duty_resolution  = led_rgb->duty_resolution,

        .timer_num        = led_rgb->timer,

        .freq_hz          = led_rgb->frequency,

        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&ledc_timer)
    );

    /* =====================================================
       CANALES RGB
       ===================================================== */
    ledc_channel_config_t channels[3] = {

        {
            .speed_mode = led_rgb->speed_mode,
            .channel    = led_rgb->led_red.channel,
            .timer_sel  = led_rgb->timer,
            .intr_type  = LEDC_INTR_DISABLE,
            .gpio_num   = led_rgb->led_red.gpio_num,
            .duty       = led_rgb->led_red.duty,
            .hpoint     = 0
        },

        {
            .speed_mode = led_rgb->speed_mode,
            .channel    = led_rgb->led_green.channel,
            .timer_sel  = led_rgb->timer,
            .intr_type  = LEDC_INTR_DISABLE,
            .gpio_num   = led_rgb->led_green.gpio_num,
            .duty       = led_rgb->led_green.duty,
            .hpoint     = 0
        },

        {
            .speed_mode = led_rgb->speed_mode,
            .channel    = led_rgb->led_blue.channel,
            .timer_sel  = led_rgb->timer,
            .intr_type  = LEDC_INTR_DISABLE,
            .gpio_num   = led_rgb->led_blue.gpio_num,
            .duty       = led_rgb->led_blue.duty,
            .hpoint     = 0
        }
    };

    for(int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(
            ledc_channel_config(&channels[i])
        );
    }
}

/* =====================================================
   CONFIGURAR BOTÓN AUXILIAR
   ===================================================== */
void config_buttons_rgb(button_rgb_t *button_rgb)
{
    gpio_config_t button_config = {

        .pin_bit_mask =
            (1ULL << button_rgb->button_aux.gpio_num),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(
        gpio_config(&button_config)
    );
}

/* =====================================================
   ENVIAR PWM A RGB
   ===================================================== */
void set_led_rgb_given_values(
    led_rgb_t *led_rgb,
    uint32_t duty_red,
    uint32_t duty_green,
    uint32_t duty_blue)
{
    /* RED */
    ledc_set_duty(
        led_rgb->speed_mode,
        led_rgb->led_red.channel,
        duty_red);

    ledc_update_duty(
        led_rgb->speed_mode,
        led_rgb->led_red.channel);

    /* GREEN */
    ledc_set_duty(
        led_rgb->speed_mode,
        led_rgb->led_green.channel,
        duty_green);

    ledc_update_duty(
        led_rgb->speed_mode,
        led_rgb->led_green.channel);

    /* BLUE */
    ledc_set_duty(
        led_rgb->speed_mode,
        led_rgb->led_blue.channel,
        duty_blue);

    ledc_update_duty(
        led_rgb->speed_mode,
        led_rgb->led_blue.channel);
}

/* =====================================================
   APAGAR RGB
   ===================================================== */
void led_rgb_off(led_rgb_t *led_rgb)
{
    uint32_t max_duty =
        (1 << led_rgb->duty_resolution) - 1;

    set_led_rgb_given_values(
        led_rgb,
        max_duty,
        max_duty,
        max_duty);
}

/* =====================================================
   ACTIVAR SOLO UN COLOR
   ===================================================== */
void led_rgb_set_single_color(
    led_rgb_t *led_rgb,
    uint32_t pwm,
    char color)
{
    uint32_t max_duty =
        (1 << led_rgb->duty_resolution) - 1;

    uint32_t red_pwm   = max_duty;

    uint32_t green_pwm = max_duty;

    uint32_t blue_pwm  = max_duty;

    switch(color)
    {
        case 'R':
            red_pwm = pwm;
            break;

        case 'G':
            green_pwm = pwm;
            break;

        case 'B':
            blue_pwm = pwm;
            break;

        default:
            break;
    }

    set_led_rgb_given_values(
        led_rgb,
        red_pwm,
        green_pwm,
        blue_pwm);
}