#include "library_led_c.h"

#include <string.h>

#include <stdlib.h>

/* =====================================================
   CONFIG PWM RGB
   ===================================================== */

void config_led_rgb(led_rgb_t *led_rgb)
{
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
   CONFIG BOTONES
   ===================================================== */

void config_buttons_rgb(button_rgb_t *button_rgb)
{
    gpio_config_t button_config = {

        .pin_bit_mask =
            (1ULL << button_rgb->button_aux.gpio_num) |

            (1ULL << button_rgb->button_temperature.gpio_num),

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
   PWM RGB
   ===================================================== */

void set_led_rgb_given_values(
    led_rgb_t *led_rgb,
    uint32_t duty_red,
    uint32_t duty_green,
    uint32_t duty_blue)
{
    ledc_set_duty(
        led_rgb->speed_mode,
        led_rgb->led_red.channel,
        duty_red);

    ledc_update_duty(
        led_rgb->speed_mode,
        led_rgb->led_red.channel);

    ledc_set_duty(
        led_rgb->speed_mode,
        led_rgb->led_green.channel,
        duty_green);

    ledc_update_duty(
        led_rgb->speed_mode,
        led_rgb->led_green.channel);

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
   COLOR ÚNICO
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
        case 'r':
            red_pwm = pwm;
            break;

        case 'g':
            green_pwm = pwm;
            break;

        case 'b':
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

/* =====================================================
   RGB MÚLTIPLE TEMPERATURA
   ===================================================== */

void set_rgb_from_temperature(
    led_rgb_t *led_rgb,
    float temperature,
    temp_range_t *ranges,
    uint32_t pwm)
{
    uint32_t max_duty =
        (1 << led_rgb->duty_resolution) - 1;

    uint32_t red_pwm = max_duty;

    uint32_t green_pwm = max_duty;

    uint32_t blue_pwm = max_duty;

    if(temperature >= ranges->red_min &&
       temperature <= ranges->red_max)
    {
        red_pwm = pwm;
    }

    if(temperature >= ranges->green_min &&
       temperature <= ranges->green_max)
    {
        green_pwm = pwm;
    }

    if(temperature >= ranges->blue_min &&
       temperature <= ranges->blue_max)
    {
        blue_pwm = pwm;
    }

    set_led_rgb_given_values(
        led_rgb,
        red_pwm,
        green_pwm,
        blue_pwm);
}

/* =====================================================
   ADC CALIBRADO
   ===================================================== */

int read_adc_calibrated_mv(
    adc_oneshot_unit_handle_t adc_handle,
    adc_cali_handle_t adc_cali_handle,
    adc_channel_t channel)
{
    int raw = 0;

    int voltage = 0;

    int samples = 16;

    int raw_sum = 0;

    for(int i = 0; i < samples; i++)
    {
        adc_oneshot_read(
            adc_handle,
            channel,
            &raw);

        raw_sum += raw;
    }

    raw = raw_sum / samples;

    adc_cali_raw_to_voltage(
        adc_cali_handle,
        raw,
        &voltage);

    return voltage;
}

/* =====================================================
   CALCULAR TEMPERATURA NTC
   ===================================================== */

float calculate_ntc_temperature(
    int voltage_mv)
{
    float voltage =
        voltage_mv / 1000.0f;

    if(voltage <= 0.0f)
    {
        return -100.0f;
    }

    float resistance_ntc =
        R_FIJA *
        ((VOLTAJE_ENTRADA / voltage) - 1.0f);

    float steinhart;

    steinhart =
        resistance_ntc / NTC_R_NOMINAL;

    steinhart = log(steinhart);

    steinhart /= NTC_BETA;

    steinhart += 1.0f / NTC_TEMP_NOMINAL;

    steinhart = 1.0f / steinhart;

    steinhart -= 273.15f;

    return steinhart;
}

/* =====================================================
   PARSE UART TEMPERATURA
   ===================================================== */

void parse_temperature_command(
    char *data,
    temp_range_t *ranges)
{
    char color;

    int min;

    int max;

    if(sscanf(data,
              "%c %d %d",
              &color,
              &min,
              &max) == 3)
    {
        switch(color)
        {
            case 'R':
            case 'r':

                ranges->red_min = min;
                ranges->red_max = max;

                printf("Rango ROJO actualizado\n");

                break;

            case 'G':
            case 'g':

                ranges->green_min = min;
                ranges->green_max = max;

                printf("Rango VERDE actualizado\n");

                break;

            case 'B':
            case 'b':

                ranges->blue_min = min;
                ranges->blue_max = max;

                printf("Rango AZUL actualizado\n");

                break;

            default:

                printf("Comando inválido\n");

                break;
        }
    }
}

/* =====================================================
   CONFIG SISTEMA UART
   ===================================================== */

void parse_system_command(
    char *data,
    int *print_interval_ms,
    char *temperature_unit)
{
    char command;

    if(sscanf(data, "%c", &command) != 1)
    {
        return;
    }

    if(command == 'T' || command == 't')
    {
        int new_time;

        if(sscanf(data,
                  "%*c %d",
                  &new_time) == 1)
        {
            *print_interval_ms = new_time;

            printf(
                "Nuevo tiempo impresión: %d ms\n",
                *print_interval_ms
            );
        }
    }

    if(command == 'U' || command == 'u')
    {
        char unit;

        if(sscanf(data,
                  "%*c %c",
                  &unit) == 1)
        {
            if(unit == 'C' || unit == 'c')
            {
                *temperature_unit = 'C';

                printf("Unidad visual: Celsius\n");
            }

            if(unit == 'K' || unit == 'k')
            {
                *temperature_unit = 'K';

                printf("Unidad visual: Kelvin\n");
            }

            if(unit == 'F' || unit == 'f')
            {
                *temperature_unit = 'F';

                printf("Unidad visual: Fahrenheit\n");
            }
        }
    }
}