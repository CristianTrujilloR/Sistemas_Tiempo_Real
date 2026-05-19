/* =====================================================
   PROYECTO:
   CONTROL RGB PWM + ADC + UART
   ESP32-C6

   FUNCIONES:
   - Control brillo mediante potenciómetro
   - Selección color por UART
   - ADC calibrado
   - PWM RGB
   - Botón auxiliar diagnóstico
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
   CONFIGURACIÓN PWM
   ===================================================== */

/* Timer PWM usado */
#define LEDC_TIMER              LEDC_TIMER_0

/* Modo PWM */
#define LEDC_MODE               LEDC_LOW_SPEED_MODE

/* Resolución:
   13 bits = 8191 */
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT

/* Frecuencia PWM */
#define LEDC_FREQUENCY          4000

/* Duty máximo PWM */
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
   ADC POTENCIÓMETRO

   GPIO0 -> ADC_CHANNEL_0
   ===================================================== */

#define POT_ADC_CHANNEL         ADC_CHANNEL_0

/* =====================================================
   UART
   ===================================================== */

#define UART_PORT               UART_NUM_0

#define UART_BUFFER_SIZE        1024

/* =====================================================
   COLOR ACTIVO ACTUAL

   Comienza en rojo
   ===================================================== */
static char active_color = 'r';

/* =====================================================
   FUNCIÓN DEBOUNCE

   Elimina rebotes mecánicos del botón
   ===================================================== */
static int button_pressed(gpio_num_t gpio)
{
    if(gpio_get_level(gpio) == 0)
    {
        /* Esperar debounce */
        vTaskDelay(pdMS_TO_TICKS(50));

        if(gpio_get_level(gpio) == 0)
        {
            /* Esperar liberación */
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
       CONFIGURACIÓN RGB
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
       CONFIGURACIÓN BOTÓN AUXILIAR
       ===================================================== */
    button_rgb_t button_rgb1 = {

        .button_aux = {
            .gpio_num = BUTTON_AUX_GPIO,
        }
    };

    /* =====================================================
       INICIALIZAR RGB Y BOTÓN
       ===================================================== */

    config_led_rgb(&led_rgb1);

    config_buttons_rgb(&button_rgb1);

    /* Comenzar con RGB apagado */
    led_rgb_off(&led_rgb1);

    /* =====================================================
       INICIALIZAR UART
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
       CONFIGURACIÓN ADC
       ===================================================== */

    adc_oneshot_unit_handle_t adc1_handle;

    adc_oneshot_unit_init_cfg_t init_config = {

        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_new_unit(
        &init_config,
        &adc1_handle
    );

    /* =====================================================
       CONFIGURACIÓN CANAL ADC
       ===================================================== */

    adc_oneshot_chan_cfg_t adc_config = {

        .bitwidth = ADC_BITWIDTH_12,

        .atten = ADC_ATTEN_DB_12,
    };

    adc_oneshot_config_channel(
        adc1_handle,
        POT_ADC_CHANNEL,
        &adc_config
    );

    /* =====================================================
       CALIBRACIÓN ADC

       Mejora precisión del voltaje
       ===================================================== */

    adc_cali_handle_t adc1_cali_handle = NULL;

    adc_cali_curve_fitting_config_t cali_config = {

        .unit_id = ADC_UNIT_1,

        .chan = POT_ADC_CHANNEL,

        .atten = ADC_ATTEN_DB_12,

        .bitwidth = ADC_BITWIDTH_12,
    };

    adc_cali_create_scheme_curve_fitting(
        &cali_config,
        &adc1_cali_handle
    );

    printf("ADC calibrado correctamente\n");

    /* =====================================================
       BUFFER UART
       ===================================================== */

    uint8_t data[20];

    while(1)
    {
        /* =====================================================
           LEER UART

           COMANDOS:
           r -> rojo
           g -> verde
           b -> azul
           off -> apagar RGB
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

            if(strstr((char*)data, "r"))
            {
                active_color = 'r';

                printf("ROJO seleccionado\n");
            }

            else if(strstr((char*)data, "g"))
            {
                active_color = 'g';

                printf("VERDE seleccionado\n");
            }

            else if(strstr((char*)data, "b"))
            {
                active_color = 'b';

                printf("AZUL seleccionado\n");
            }

            else if(strstr((char*)data, "off"))
            {
                led_rgb_off(&led_rgb1);

                printf("RGB apagado\n");
            }
        }

        /* =====================================================
           LEER ADC DEL POTENCIÓMETRO
           ===================================================== */

        int adc_raw = 0;

        adc_oneshot_read(
            adc1_handle,
            POT_ADC_CHANNEL,
            &adc_raw
        );

        /* =====================================================
           CONVERTIR ADC A VOLTAJE REAL CALIBRADO
           ===================================================== */

        int voltage_mv = 0;

        adc_cali_raw_to_voltage(
            adc1_cali_handle,
            adc_raw,
            &voltage_mv
        );

        /* Convertir mV -> V */
        float voltage =
            voltage_mv / 1000.0;

        /* =====================================================
           CALCULAR PWM

           RGB ÁNODO COMÚN:
           duty bajo = más brillo
           duty alto = menos brillo
           ===================================================== */

        uint32_t pwm =
            PWM_MAX_DUTY -
            ((voltage_mv * PWM_MAX_DUTY) / 3300);

        /* =====================================================
           ACTUALIZAR COLOR ACTIVO
           ===================================================== */

        led_rgb_set_single_color(
            &led_rgb1,
            pwm,
            active_color
        );

        /* =====================================================
           BOTÓN AUXILIAR

           Muestra:
           - color activo
           - ADC RAW
           - voltaje
           - PWM enviado
           ===================================================== */

        if(button_pressed(button_rgb1.button_aux.gpio_num))
        {
            printf("\n");

            printf("========== ADC INFO ==========\n");

            printf("Color activo : %c\n", active_color);

            printf("ADC RAW      : %d\n", adc_raw);

            printf("Voltaje      : %.3f V\n", voltage);

            printf("PWM enviado  : %lu\n", pwm);

            printf("==============================\n\n");
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}