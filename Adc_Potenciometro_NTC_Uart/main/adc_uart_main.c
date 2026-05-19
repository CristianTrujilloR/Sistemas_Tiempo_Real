/* =====================================================
   PROYECTO:
   TERMISTOR NTC + RGB + UART
   ESP32-C6

   FUNCIONES:
   - Lectura temperatura
   - ADC calibrado
   - Control RGB automático
   - Rangos configurables UART

   COMANDOS UART:

   R 0 15
   G 16 30
   B 31 50

   INFO

   ===================================================== */

#include <stdio.h>

#include <string.h>

#include <math.h>

#include "driver/uart.h"

#include "esp_adc/adc_oneshot.h"

#include "esp_adc/adc_cali.h"

#include "esp_adc/adc_cali_scheme.h"

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"

#include "library_led_c.h"
/* =====================================================
   CONFIGURACIÓN PWM
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
   BOTÓN AUXILIAR
   ===================================================== */

#define BUTTON_AUX_GPIO         GPIO_NUM_22

/* =====================================================
   ADC TERMISTOR
   ===================================================== */

#define NTC_ADC_CHANNEL         ADC_CHANNEL_0

/* =====================================================
   UART
   ===================================================== */

#define UART_PORT               UART_NUM_0

#define UART_BUFFER_SIZE        1024

/* =====================================================
   PARÁMETROS NTC
   ===================================================== */

#define R_FIJA             100000.0f

#define NTC_R_NOMINAL      100000.0f

#define NTC_TEMP_NOMINAL   298.15f

#define NTC_BETA           4190.0f

#define VOLTAGE_ENTRADA    3.3f

/* =====================================================
   RANGOS TEMPERATURA

   Valores iniciales
   ===================================================== */

static int red_min   = 0;
static int red_max   = 15;

static int green_min = 16;
static int green_max = 30;

static int blue_min  = 31;
static int blue_max  = 50;

/* =====================================================
   DEBOUNCE BOTÓN
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
       CONFIG BOTÓN
       ===================================================== */

    button_rgb_t button_rgb1 = {

        .button_aux = {
            .gpio_num = BUTTON_AUX_GPIO,
        }
    };

    /* =====================================================
       INIT RGB
       ===================================================== */

    config_led_rgb(&led_rgb1);

    config_buttons_rgb(&button_rgb1);

    led_rgb_off(&led_rgb1);

    /* =====================================================
       UART INIT
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
       ADC INIT
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

    /* =====================================================
       ADC CALIBRACIÓN
       ===================================================== */

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

    printf("Sistema iniciado\n");

    uint8_t data[50];

    while(1)
    {
        /* =====================================================
           LEER UART
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

            char color;

            int min;

            int max;

            if(sscanf((char*)data, "%c %d %d",
                &color,
                &min,
                &max) == 3)
            {
                switch(color)
                {
                    case 'R':
                        red_min = min;
                        red_max = max;
                        break;

                    case 'G':
                        green_min = min;
                        green_max = max;
                        break;

                    case 'B':
                        blue_min = min;
                        blue_max = max;
                        break;

                    default:
                        break;
                }

                printf("\n");
                printf("Rangos actualizados\n");
            }

            if(strstr((char*)data, "INFO"))
            {
                printf("\n");

                printf("R: %d -> %d\n",
                    red_min,
                    red_max);

                printf("G: %d -> %d\n",
                    green_min,
                    green_max);

                printf("B: %d -> %d\n",
                    blue_min,
                    blue_max);

                printf("\n");
            }
        }

        /* =====================================================
           LEER ADC
           ===================================================== */

        int adc_raw = 0;

        adc_oneshot_read(
            adc1_handle,
            NTC_ADC_CHANNEL,
            &adc_raw
        );

        /* =====================================================
           ADC -> VOLTAJE
           ===================================================== */

        int voltage_mv = 0;

        adc_cali_raw_to_voltage(
            adc1_cali_handle,
            adc_raw,
            &voltage_mv
        );

        float voltage =
            voltage_mv / 1000.0f;

        /* =====================================================
           CALCULAR RESISTENCIA NTC

           DIVISOR:
           NTC arriba
           resistencia fija abajo
           ===================================================== */

        float resistance_ntc =
            R_FIJA *
            (voltage /
            (VOLTAGE_ENTRADA - voltage));

        /* =====================================================
           ECUACIÓN BETA
           ===================================================== */

        float temperature_kelvin =
            1.0f /
            (
                (1.0f / NTC_TEMP_NOMINAL) +

                (1.0f / NTC_BETA) *

                logf(
                    resistance_ntc /
                    NTC_R_NOMINAL
                )
            );

        /* Kelvin -> Celsius */
        float temperature_c =
            temperature_kelvin - 273.15f;

        /* =====================================================
           CONTROL RGB AUTOMÁTICO
           ===================================================== */

        if(temperature_c >= red_min &&
           temperature_c <= red_max)
        {
            led_rgb_set_single_color(
                &led_rgb1,
                0,
                'R');
        }

        else if(temperature_c >= green_min &&
                temperature_c <= green_max)
        {
            led_rgb_set_single_color(
                &led_rgb1,
                0,
                'G');
        }

        else if(temperature_c >= blue_min &&
                temperature_c <= blue_max)
        {
            led_rgb_set_single_color(
                &led_rgb1,
                0,
                'B');
        }

        else
        {
            led_rgb_off(&led_rgb1);
        }

        /* =====================================================
           BOTÓN AUXILIAR

           Muestra información completa
           ===================================================== */

        if(button_pressed(button_rgb1.button_aux.gpio_num))
        {
            printf("\n");

            printf("========== NTC INFO ==========\n");

            printf("ADC RAW      : %d\n", adc_raw);

            printf("Voltaje      : %.3f V\n", voltage);

            printf("Resistencia  : %.2f ohms\n",
                resistance_ntc);

            printf("Temperatura  : %.2f C\n",
                temperature_c);

            printf("==============================\n\n");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}