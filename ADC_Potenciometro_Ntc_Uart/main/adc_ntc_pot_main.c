/* =====================================================
   PROYECTO:
   RGB PWM + NTC ADC + UART
   ESP32-C6

   FUNCIONES:
   - NTC controla color automáticamente
   - Mezcla colores RGB
   - UART configura rangos térmicos
   - UART controla intensidad PWM
   - ADC calibrado
   - PWM RGB
   - Botón diagnóstico
   - Botón cambio temperatura
   - Impresión automática temperatura
   - Celsius / Kelvin / Fahrenheit

   ===================================================== */

#include <stdio.h>

#include <string.h>

#include "driver/uart.h"

#include "esp_adc/adc_oneshot.h"

#include "esp_adc/adc_cali.h"

#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"

#include "library_led_c.h"

/* =====================================================
   CONFIG PWM
   ===================================================== */

#define LEDC_TIMER              LEDC_TIMER_0

#define LEDC_MODE               LEDC_LOW_SPEED_MODE

#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT

#define LEDC_FREQUENCY          4000

#define PWM_MAX_DUTY ((1 << LEDC_DUTY_RES) - 1)

/* =====================================================
   GPIO RGB
   ===================================================== */

#define LED_RGB1_RED_GPIO       GPIO_NUM_2

#define LED_RGB1_GREEN_GPIO     GPIO_NUM_4

#define LED_RGB1_BLUE_GPIO      GPIO_NUM_5

/* =====================================================
   BOTONES
   ===================================================== */

#define BUTTON_AUX_GPIO         GPIO_NUM_22

#define BUTTON_TEMPERATURE_GPIO GPIO_NUM_21

/* =====================================================
   ADC
   ===================================================== */

#define NTC_ADC_CHANNEL         ADC_CHANNEL_1

/* =====================================================
   UART
   ===================================================== */

#define UART_PORT               UART_NUM_0

#define UART_BUFFER_SIZE        1024

/* =====================================================
   RANGOS TEMPERATURA
   ===================================================== */

temp_range_t temp_ranges = {

    .red_min = 0,
    .red_max = 15,

    .green_min = 16,
    .green_max = 30,

    .blue_min = 31,
    .blue_max = 50
};

/* =====================================================
   DEBOUNCE
   ===================================================== */

static int button_pressed(gpio_num_t gpio)
{
    if(gpio_get_level(gpio) == 0)
    {
        vTaskDelay(pdMS_TO_TICKS(50));

        if(gpio_get_level(gpio) == 0)
        {
            while(gpio_get_level(gpio) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            return 1;
        }
    }

    return 0;
}

/* =====================================================
   APP MAIN
   ===================================================== */

void app_main(void)
{
    /* =====================================================
       CONFIG RGB
       ===================================================== */

    led_rgb_t led_rgb1 = {

        .led_red = {
            .duty = PWM_MAX_DUTY,
            .gpio_num = LED_RGB1_RED_GPIO,
            .channel = LEDC_CHANNEL_0,
        },

        .led_green = {
            .duty = PWM_MAX_DUTY,
            .gpio_num = LED_RGB1_GREEN_GPIO,
            .channel = LEDC_CHANNEL_1,
        },

        .led_blue = {
            .duty = PWM_MAX_DUTY,
            .gpio_num = LED_RGB1_BLUE_GPIO,
            .channel = LEDC_CHANNEL_2,
        },

        .timer = LEDC_TIMER,

        .frequency = LEDC_FREQUENCY,

        .duty_resolution = LEDC_DUTY_RES,

        .speed_mode = LEDC_MODE
    };

    /* =====================================================
       CONFIG BOTONES
       ===================================================== */

    button_rgb_t button_rgb1 = {

        .button_aux = {
            .gpio_num = BUTTON_AUX_GPIO,
        },

        .button_temperature = {
            .gpio_num = BUTTON_TEMPERATURE_GPIO,
        }
    };

    config_led_rgb(&led_rgb1);

    config_buttons_rgb(&button_rgb1);

    led_rgb_off(&led_rgb1);

    /* =====================================================
       UART
       ===================================================== */

    uart_driver_install(
        UART_PORT,
        UART_BUFFER_SIZE,
        0,
        0,
        NULL,
        0
    );

    /* =====================================================
       ADC
       ===================================================== */

    adc_oneshot_unit_handle_t adc1_handle;

    adc_oneshot_unit_init_cfg_t init_config = {

        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_new_unit(
        &init_config,
        &adc1_handle
    );

    adc_oneshot_chan_cfg_t adc_config = {

        .bitwidth = ADC_BITWIDTH_12,

        .atten = ADC_ATTEN_DB_12,
    };

    adc_oneshot_config_channel(
        adc1_handle,
        NTC_ADC_CHANNEL,
        &adc_config
    );

    adc_cali_handle_t adc1_cali_handle = NULL;

    adc_cali_curve_fitting_config_t cali_config = {

        .unit_id = ADC_UNIT_1,

        .chan = NTC_ADC_CHANNEL,

        .atten = ADC_ATTEN_DB_12,

        .bitwidth = ADC_BITWIDTH_12,
    };

    adc_cali_create_scheme_curve_fitting(
        &cali_config,
        &adc1_cali_handle
    );

    printf("ADC calibrado correctamente\n");

    uint8_t data[50];

    /* =====================================================
       CONFIG IMPRESIÓN
       ===================================================== */

    int print_interval_ms = 3000;

    char temperature_unit = 'C';

    TickType_t last_print_time = 0;

    /* =====================================================
       PWM UART
       ===================================================== */

    int pwm_percentage = 100;

    while(1)
    {
        /* =====================================================
           UART
           ===================================================== */

        int len = uart_read_bytes(
            UART_PORT,
            data,
            sizeof(data) - 1,
            pdMS_TO_TICKS(10)
        );

        if(len > 0)
        {
            data[len] = 0;

            parse_temperature_command(
                (char*)data,
                &temp_ranges
            );

            parse_system_command(
                (char*)data,
                &print_interval_ms,
                &temperature_unit
            );

            /* =====================================================
               INTENSIDAD PWM

               I 0 -> OFF
               I 1 -> 20%
               I 2 -> 40%
               I 3 -> 60%
               I 4 -> 80%
               I 5 -> 100%
               ===================================================== */

            char command;

            int level;

            if(sscanf((char*)data,
                      "%c %d",
                      &command,
                      &level) == 2)
            {
                if(command == 'I' ||
                   command == 'i')
                {
                    switch(level)
                    {
                        case 0:
                            pwm_percentage = 0;
                            printf("PWM 0%%\n");
                            break;

                        case 1:
                            pwm_percentage = 20;
                            printf("PWM 20%%\n");
                            break;

                        case 2:
                            pwm_percentage = 40;
                            printf("PWM 40%%\n");
                            break;

                        case 3:
                            pwm_percentage = 60;
                            printf("PWM 60%%\n");
                            break;

                        case 4:
                            pwm_percentage = 80;
                            printf("PWM 80%%\n");
                            break;

                        case 5:
                            pwm_percentage = 100;
                            printf("PWM 100%%\n");
                            break;

                        default:
                            printf("Nivel inválido\n");
                            break;
                    }
                }
            }
        }

        /* =====================================================
           BOTÓN TEMPERATURA

           Celsius -> Kelvin -> Fahrenheit
           ===================================================== */

        if(button_pressed(
            button_rgb1.button_temperature.gpio_num))
        {
            if(temperature_unit == 'C')
            {
                temperature_unit = 'K';

                printf("Unidad: Kelvin\n");
            }
            else if(temperature_unit == 'K')
            {
                temperature_unit = 'F';

                printf("Unidad: Fahrenheit\n");
            }
            else
            {
                temperature_unit = 'C';

                printf("Unidad: Celsius\n");
            }
        }

        /* =====================================================
           LEER NTC
           ===================================================== */

        int ntc_mv = read_adc_calibrated_mv(
            adc1_handle,
            adc1_cali_handle,
            NTC_ADC_CHANNEL
        );

        /* =====================================================
           TEMPERATURA REAL CELSIUS
           ===================================================== */

        float temperature =
            calculate_ntc_temperature(ntc_mv);

        /* =====================================================
           TEMPERATURA VISUAL
           ===================================================== */

        float display_temperature = temperature;

        if(temperature_unit == 'K')
        {
            display_temperature =
                temperature + 273.15f;
        }

        if(temperature_unit == 'F')
        {
            display_temperature =
                (temperature * 1.8f) + 32.0f;
        }

        /* =====================================================
           IMPRESIÓN AUTOMÁTICA
           ===================================================== */

        if((xTaskGetTickCount() - last_print_time) >=
            pdMS_TO_TICKS(print_interval_ms))
        {
            printf("\n");

            printf("======= TEMPERATURA =======\n");

            printf(
                "Temperatura: %.2f %c\n",
                display_temperature,
                temperature_unit
            );

            printf("===========================\n\n");

            last_print_time =
                xTaskGetTickCount();
        }

        /* =====================================================
           PWM UART
           ===================================================== */

        uint32_t pwm =
            PWM_MAX_DUTY -
            ((pwm_percentage *
            PWM_MAX_DUTY) / 100);

        /* =====================================================
           RGB DINÁMICO

           Mezcla colores automáticamente
           ===================================================== */

        set_rgb_from_temperature(
            &led_rgb1,
            temperature,
            &temp_ranges,
            pwm
        );

        /* =====================================================
           BOTÓN DIAGNÓSTICO
           ===================================================== */

        if(button_pressed(button_rgb1.button_aux.gpio_num))
        {
            printf("\n");

            printf("=========== NTC INFO ===========\n");

            printf(
                "Temperatura : %.2f %c\n",
                display_temperature,
                temperature_unit
            );

            printf("NTC mV      : %d\n",
                   ntc_mv);

            printf("RGB dinámico activo\n");

            printf("PWM %%       : %d\n",
                   pwm_percentage);

            printf("RANGO R     : %d - %d\n",
                   temp_ranges.red_min,
                   temp_ranges.red_max);

            printf("RANGO G     : %d - %d\n",
                   temp_ranges.green_min,
                   temp_ranges.green_max);

            printf("RANGO B     : %d - %d\n",
                   temp_ranges.blue_min,
                   temp_ranges.blue_max);

            printf("Tiempo ms   : %d\n",
                   print_interval_ms);

            printf("Unidad      : %c\n",
                   temperature_unit);

            printf("================================\n\n");
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}