#include "rgb_led.h"

#include "driver/ledc.h"

#define RGB_RED_GPIO      25
#define RGB_GREEN_GPIO    26
#define RGB_BLUE_GPIO     27

#define RGB_TIMER         LEDC_TIMER_1
#define RGB_MODE          LEDC_LOW_SPEED_MODE

#define RGB_RED_CHANNEL   LEDC_CHANNEL_1
#define RGB_GREEN_CHANNEL LEDC_CHANNEL_2
#define RGB_BLUE_CHANNEL  LEDC_CHANNEL_3

#define RGB_FREQ          5000
#define RGB_RES           LEDC_TIMER_8_BIT

static uint8_t rgb_r = 255;
static uint8_t rgb_g = 255;
static uint8_t rgb_b = 255;

static uint8_t brightness = 100;

/* ===================================================== */
/* Internal Apply                                         */
/* ===================================================== */

static void rgb_led_apply(void)
{
    uint32_t r =
        (rgb_r * brightness) / 100;

    uint32_t g =
        (rgb_g * brightness) / 100;

    uint32_t b =
        (rgb_b * brightness) / 100;

    /*
     * Common Anode RGB
     * PWM inverted
     */

    r = 255 - r;
    g = 255 - g;
    b = 255 - b;

    ledc_set_duty(
        RGB_MODE,
        RGB_RED_CHANNEL,
        r);

    ledc_update_duty(
        RGB_MODE,
        RGB_RED_CHANNEL);

    ledc_set_duty(
        RGB_MODE,
        RGB_GREEN_CHANNEL,
        g);

    ledc_update_duty(
        RGB_MODE,
        RGB_GREEN_CHANNEL);

    ledc_set_duty(
        RGB_MODE,
        RGB_BLUE_CHANNEL,
        b);

    ledc_update_duty(
        RGB_MODE,
        RGB_BLUE_CHANNEL);
}

/* ===================================================== */
/* Init                                                   */
/* ===================================================== */

void rgb_led_init(void)
{
    ledc_timer_config_t timer =
    {
        .speed_mode = RGB_MODE,
        .timer_num = RGB_TIMER,
        .duty_resolution = RGB_RES,
        .freq_hz = RGB_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer);

    ledc_channel_config_t red =
    {
        .gpio_num = RGB_RED_GPIO,
        .speed_mode = RGB_MODE,
        .channel = RGB_RED_CHANNEL,
        .timer_sel = RGB_TIMER,
        .duty = 255,
        .hpoint = 0
    };

    ledc_channel_config(&red);

    ledc_channel_config_t green =
    {
        .gpio_num = RGB_GREEN_GPIO,
        .speed_mode = RGB_MODE,
        .channel = RGB_GREEN_CHANNEL,
        .timer_sel = RGB_TIMER,
        .duty = 255,
        .hpoint = 0
    };

    ledc_channel_config(&green);

    ledc_channel_config_t blue =
    {
        .gpio_num = RGB_BLUE_GPIO,
        .speed_mode = RGB_MODE,
        .channel = RGB_BLUE_CHANNEL,
        .timer_sel = RGB_TIMER,
        .duty = 255,
        .hpoint = 0
    };

    ledc_channel_config(&blue);

    rgb_led_apply();
}

/* ===================================================== */
/* Public                                                 */
/* ===================================================== */

void rgb_led_set_color(
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    rgb_r = red;
    rgb_g = green;
    rgb_b = blue;

    rgb_led_apply();
}

void rgb_led_set_brightness(uint8_t percent)
{
    if(percent > 100)
    {
        percent = 100;
    }

    brightness = percent;

    rgb_led_apply();
}

void rgb_led_wifi_app_started(void)
{
    /* Amarillo */
    rgb_led_set_color(255,255,0);
    rgb_led_set_brightness(20);
}

void rgb_led_wifi_connected(void)
{
    /* Verde */
    rgb_led_set_color(0,255,0);
    rgb_led_set_brightness(40);
}

void rgb_led_http_server_started(void)
{
    rgb_led_set_color(255,0,0);
    rgb_led_set_brightness(100);
}
































