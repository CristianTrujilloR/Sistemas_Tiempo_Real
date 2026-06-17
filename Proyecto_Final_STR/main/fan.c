#include "fan.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define FAN_GPIO        18
#define ALARM_LED_GPIO  4

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_FREQ       5000
#define LEDC_RES        LEDC_TIMER_10_BIT

static uint8_t fan_mode = 0;              // 0 AUTO, 1 MANUAL
static uint8_t fan_manual_percent = 0;

static uint8_t alarm_active = 0;
static TaskHandle_t alarm_task_handle = NULL;

/* ================= PWM FAN ================= */
void fan_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_RES,
        .freq_hz = LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num = FAN_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel);

    /* LED alarma */
    gpio_reset_pin(ALARM_LED_GPIO);
    gpio_set_direction(ALARM_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(ALARM_LED_GPIO, 0);
}

/* ================= FAN PWM ================= */
static void fan_set_duty(float percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    printf("Fan Duty = %.1f %%\n", percent);

    uint32_t duty = (uint32_t)((percent / 100.0f) * 1023);

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
/* ================= ALARM TASK (1Hz blink) ================= */
static void alarm_task(void *arg)
{
    while (1)
    {
        if (alarm_active)
        {
            gpio_set_level(ALARM_LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(500));

            gpio_set_level(ALARM_LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            gpio_set_level(ALARM_LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

/* ================= INIT TASK ================= */
static void alarm_init_task(void)
{
    xTaskCreate(alarm_task, "alarm_task", 2048, NULL, 5, &alarm_task_handle);
}

/* ================= PUBLIC ================= */
void fan_set_manual(uint8_t percent)
{
    fan_mode = 1;

    if (percent > 100) percent = 100;

    fan_manual_percent = percent;
    fan_set_duty(percent);
}

void fan_set_mode(uint8_t mode)
{
    fan_mode = mode;
}

/* ================= AUTO CONTROL ================= */
void fan_update_auto(float temp, float t_desired, float t_max)
{
    float duty = 0;

    /* ===== ALARMA ===== */
    if (temp >= t_max)
    {
        alarm_active = 1;
    }
    else
    {
        alarm_active = 0;
    }

    /* ===== MANUAL ===== */
    if (fan_mode == 1)
    {
        fan_set_duty(fan_manual_percent);
        return;
    }

    /* ===== AUTO FAN ===== */
    if (temp <= t_desired)
    {
        duty = 0;
    }
    else if (temp >= t_max)
    {
        duty = 100;
    }
    else
    {
        duty = 40.0f +
               (temp - t_desired) *
               60.0f /
               (t_max - t_desired);
    }

    fan_set_duty(duty);
}

uint8_t fan_get_mode(void)
{
    return fan_mode;
}

/* ================= PUBLIC INIT WRAPPER ================= */
void fan_start(void)
{
    fan_init();
    alarm_init_task();
}