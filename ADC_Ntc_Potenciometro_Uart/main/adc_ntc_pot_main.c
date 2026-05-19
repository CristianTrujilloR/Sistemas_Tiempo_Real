/* =====================================================
   PROYECTO:
   RGB PWM + POT ADC + NTC ADC + UART
   ESP32-C6

   FUNCIONES:
   - Potenciómetro controla brillo PWM
   - NTC controla color automáticamente
   - UART configura rangos térmicos
   - ADC calibrado
   - PWM RGB
   - Botón diagnóstico
   - Corrección transición térmica RGB
   - Conversión correcta NTC 100k Beta 4190

   CONEXIÓN NTC:

          3.3V
            |
          [NTC]
            |
            +---- GPIO1 ADC
            |
         [100k]
            |
           GND

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
   ADC POTENCIÓMETRO

   GPIO0 -> ADC_CHANNEL_0
   ===================================================== */

#define POT_ADC_CHANNEL         ADC_CHANNEL_0

/* =====================================================
   ADC NTC

   GPIO1 -> ADC_CHANNEL_1
   ===================================================== */

#define NTC_ADC_CHANNEL         ADC_CHANNEL_1

/* =====================================================
   UART
   ===================================================== */

#define UART_PORT               UART_NUM_0

#define UART_BUFFER_SIZE        1024

/* =====================================================
   RANGOS TEMPERATURA

   Valores iniciales
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
   FUNCIÓN DEBOUNCE

   Elimina rebotes mecánicos
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
       CONFIGURACIÓN BOTÓN
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

    led_rgb_off(&led_rgb1);

    /* =====================================================
       CONFIGURAR UART
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
       CONFIGURAR ADC
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
       CONFIGURAR CANALES ADC
       ===================================================== */

    adc_oneshot_chan_cfg_t adc_config = {

        .bitwidth = ADC_BITWIDTH_12,

        .atten = ADC_ATTEN_DB_12,
    };

    /* POT */
    adc_oneshot_config_channel(
        adc1_handle,
        POT_ADC_CHANNEL,
        &adc_config
    );

    /* NTC */
    adc_oneshot_config_channel(
        adc1_handle,
        NTC_ADC_CHANNEL,
        &adc_config
    );

    /* =====================================================
       CALIBRACIÓN ADC
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

    uint8_t data[50];

    /* =====================================================
       VARIABLES COLOR

       current_color:
       color actual

       previous_color:
       detecta transición térmica
       ===================================================== */

    char current_color = 'r';

    char previous_color = 'x';

    while(1)
    {
        /* =====================================================
           UART CONFIG RANGOS

           FORMATO:

           R 0 15
           G 16 30
           B 31 50
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
        }

        /* =====================================================
           LEER POTENCIÓMETRO

           Controla brillo PWM
           ===================================================== */

        int pot_mv = read_adc_calibrated_mv(
            adc1_handle,
            adc1_cali_handle,
            POT_ADC_CHANNEL
        );

        /* =====================================================
           LEER NTC

           Voltaje calibrado
           ===================================================== */

        int ntc_mv = read_adc_calibrated_mv(
            adc1_handle,
            adc1_cali_handle,
            NTC_ADC_CHANNEL
        );
        /* =====================================================
           CALCULAR TEMPERATURA

           Conversión:
           - Beta 4190
           - NTC 100k
           - Steinhart-Hart
           ===================================================== */

        float temperature =
            calculate_ntc_temperature(ntc_mv);

        /* =====================================================
           OBTENER COLOR SEGÚN TEMPERATURA
           ===================================================== */

        current_color =
            get_color_from_temperature(
                temperature,
                &temp_ranges
            );

        /* =====================================================
           PWM DESDE POTENCIÓMETRO

           RGB ÁNODO COMÚN:
           duty bajo = más brillo
           ===================================================== */

        uint32_t pwm =
            PWM_MAX_DUTY -
            ((pot_mv * PWM_MAX_DUTY) / 3300);

        /* =====================================================
           DETECTAR CAMBIO DE COLOR

           Evita apagado momentáneo
           ===================================================== */

        if(current_color != previous_color)
        {
            led_rgb_off(&led_rgb1);

            vTaskDelay(pdMS_TO_TICKS(5));

            previous_color = current_color;
        }

        /* =====================================================
           ACTUALIZAR RGB
           ===================================================== */

        led_rgb_set_single_color(
            &led_rgb1,
            pwm,
            current_color
        );

        /* =====================================================
           BOTÓN DIAGNÓSTICO
           ===================================================== */

        if(button_pressed(button_rgb1.button_aux.gpio_num))
        {
            printf("\n");

            printf("=========== NTC INFO ===========\n");

            printf("Temperatura : %.2f C\n",
                   temperature);

            printf("NTC mV      : %d\n",
                   ntc_mv);

            printf("POT mV      : %d\n",
                   pot_mv);

            printf("Color       : %c\n",
                   current_color);

            printf("PWM         : %lu\n",
                   pwm);

            printf("RANGO R     : %d - %d\n",
                   temp_ranges.red_min,
                   temp_ranges.red_max);

            printf("RANGO G     : %d - %d\n",
                   temp_ranges.green_min,
                   temp_ranges.green_max);

            printf("RANGO B     : %d - %d\n",
                   temp_ranges.blue_min,
                   temp_ranges.blue_max);

            printf("================================\n\n");
        }

        /* =====================================================
           DELAY PRINCIPAL
           ===================================================== */

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}