#include "servo_motor.h"

#include "driver/ledc.h"

#define SERVO_GPIO      19

#define SERVO_TIMER     LEDC_TIMER_2
#define SERVO_MODE      LEDC_LOW_SPEED_MODE
#define SERVO_CHANNEL   LEDC_CHANNEL_4

#define SERVO_FREQ      50
#define SERVO_RES       LEDC_TIMER_16_BIT

static uint8_t current_percent = 0;

/* ===================================================== */
/* Percent -> Duty                                        */
/* ===================================================== */

static uint32_t servo_percent_to_duty(uint8_t percent)
{
    if(percent > 100)
    {
        percent = 100;
    }

    /*
     * SG90:
     *
     * 0°   -> 500us
     * 90°  -> 1500us
     * 180° -> 2500us
     */

    float pulse_us =
        500.0f +
        ((float)percent * 2000.0f / 100.0f);

    float period_us = 20000.0f;

    uint32_t duty =
        (uint32_t)(
            (pulse_us / period_us)
            * 65535.0f);

    return duty;
}

/* ===================================================== */
/* Init                                                   */
/* ===================================================== */

void servo_init(void)
{
    ledc_timer_config_t timer =
    {
        .speed_mode = SERVO_MODE,
        .timer_num = SERVO_TIMER,
        .duty_resolution = SERVO_RES,
        .freq_hz = SERVO_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer);

    ledc_channel_config_t channel =
    {
        .gpio_num = SERVO_GPIO,
        .speed_mode = SERVO_MODE,
        .channel = SERVO_CHANNEL,
        .timer_sel = SERVO_TIMER,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config(&channel);

    servo_set_percent(0);
}

/* ===================================================== */
/* Position                                               */
/* ===================================================== */

void servo_set_percent(uint8_t percent)
{
    if(percent > 100)
    {
        percent = 100;
    }

    current_percent = percent;

    uint32_t duty =
        servo_percent_to_duty(percent);

    ledc_set_duty(
        SERVO_MODE,
        SERVO_CHANNEL,
        duty);

    ledc_update_duty(
        SERVO_MODE,
        SERVO_CHANNEL);

    printf(
        "Servo Position = %d%%\n",
        percent);
}

uint8_t servo_get_percent(void)
{
    return current_percent;
}