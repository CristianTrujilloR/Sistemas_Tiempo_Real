/* LEDC (LED Controller) basic example */

#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "library_led_c.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY          (4000)

/* RGB ÁNODO COMÚN
   LOW  = ENCENDER
   HIGH = APAGAR
*/

#define LED_RGB1_RED_GPIO       GPIO_NUM_2
#define LED_RGB1_GREEN_GPIO     GPIO_NUM_4
#define LED_RGB1_BLUE_GPIO      GPIO_NUM_5

// Botones colores
#define BUTTON_RGB1_RED_GPIO    GPIO_NUM_18
#define BUTTON_RGB1_GREEN_GPIO  GPIO_NUM_19
#define BUTTON_RGB1_BLUE_GPIO   GPIO_NUM_21

// Nuevo botón RESET
#define BUTTON_RGB1_RESET_GPIO  GPIO_NUM_22

#define BUTTON_PRESSED_LEVEL    0
#define BUTTON_DEBOUNCE_TIME_MS 50
#define BUTTON_RELEASE_TIME_MS  10

#define LED_PERCENTAGE_STEP     10
#define LED_PERCENTAGE_MAX      100

/* =====================================================
   Detecta si el botón fue presionado correctamente
   con debounce
   ===================================================== */
static int button_pressed(gpio_num_t button_gpio)
{
    if (gpio_get_level(button_gpio) == BUTTON_PRESSED_LEVEL)
    {
        vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_TIME_MS));

        if (gpio_get_level(button_gpio) == BUTTON_PRESSED_LEVEL)
        {
            while (gpio_get_level(button_gpio) == BUTTON_PRESSED_LEVEL)
            {
                vTaskDelay(pdMS_TO_TICKS(BUTTON_RELEASE_TIME_MS));
            }

            return 1;
        }
    }

    return 0;
}

/* =====================================================
   Aumenta el brillo de 10% en 10%
   Si pasa de 100 vuelve a 0
   ===================================================== */
static int increase_percentage(int percentage)
{
    percentage += LED_PERCENTAGE_STEP;

    if (percentage > LED_PERCENTAGE_MAX)
    {
        percentage = 0;
    }

    return percentage;
}

void app_main(void)
{
    /* =====================================================
       Duty máximo para 13 bits:
       2^13 - 1 = 8191
       ===================================================== */

    uint32_t max_duty = (1 << LEDC_DUTY_RES) - 1;

    /* =====================================================
       Configuración RGB
       ===================================================== */

    led_rgb_t led_rgb1 = {

        .led_red = {
            .duty = max_duty,
            .gpio_num = LED_RGB1_RED_GPIO,
            .channel = LEDC_CHANNEL_0,
        },

        .led_green = {
            .duty = max_duty,
            .gpio_num = LED_RGB1_GREEN_GPIO,
            .channel = LEDC_CHANNEL_1,
        },

        .led_blue = {
            .duty = max_duty,
            .gpio_num = LED_RGB1_BLUE_GPIO,
            .channel = LEDC_CHANNEL_2,
        },

        .timer = LEDC_TIMER,
        .frequency = LEDC_FREQUENCY,
        .duty_resolution = LEDC_DUTY_RES,
        .speed_mode = LEDC_MODE
    };

    /* =====================================================
       Configuración botones
       ===================================================== */

    button_rgb_t button_rgb1 = {

        .button_red = {
            .gpio_num = BUTTON_RGB1_RED_GPIO,
        },

        .button_green = {
            .gpio_num = BUTTON_RGB1_GREEN_GPIO,
        },

        .button_blue = {
            .gpio_num = BUTTON_RGB1_BLUE_GPIO,
        },

        // Nuevo botón reset
        .button_reset = {
            .gpio_num = BUTTON_RGB1_RESET_GPIO,
        },
    };

    /* =====================================================
       Variables porcentaje brillo
       ===================================================== */

    int percentage_red = 0;
    int percentage_green = 0;
    int percentage_blue = 0;

    /* =====================================================
       Inicializar RGB y botones
       ===================================================== */

    config_led_rgb(&led_rgb1);

    config_buttons_rgb(&button_rgb1);

    /* =====================================================
       Comenzar con LED apagado
       ===================================================== */

    set_led_rgb_percentage_given_values(
        &led_rgb1,
        percentage_red,
        percentage_green,
        percentage_blue
    );

    while(1)
    {
        /* =====================================================
           BOTÓN ROJO
           ===================================================== */
        if (button_pressed(button_rgb1.button_red.gpio_num))
        {
            printf("Boton rojo presionado\n");
            printf("porcentaje rojo actual: %d\n", percentage_red);

            // Apaga otros colores
            percentage_green = 0;
            percentage_blue = 0;

            // Aumenta rojo
            percentage_red =
                increase_percentage(percentage_red);

            // Actualiza RGB
            set_led_rgb_percentage_given_values(
                &led_rgb1,
                percentage_red,
                percentage_green,
                percentage_blue
            );
        }

        /* =====================================================
           BOTÓN VERDE
           ===================================================== */
        if (button_pressed(button_rgb1.button_green.gpio_num))
        {
            printf("Boton verde presionado\n");
            printf("porcentaje_green: %d\n", percentage_green);

            // Apaga otros colores
            percentage_red = 0;
            percentage_blue = 0;

            // Aumenta verde
            percentage_green =
                increase_percentage(percentage_green);

            // Actualiza RGB
            set_led_rgb_percentage_given_values(
                &led_rgb1,
                percentage_red,
                percentage_green,
                percentage_blue
            );
        }

        /* =====================================================
           BOTÓN AZUL
           ===================================================== */
        if (button_pressed(button_rgb1.button_blue.gpio_num))
        {
            printf("Boton azul presionado\n");
            printf("porcentaje azul actual: %d\n", percentage_blue);

            // Apaga otros colores
            percentage_red = 0;
            percentage_green = 0;

            // Aumenta azul
            percentage_blue =
                increase_percentage(percentage_blue);

            // Actualiza RGB
            set_led_rgb_percentage_given_values(
                &led_rgb1,
                percentage_red,
                percentage_green,
                percentage_blue
            );
        }

        /* =====================================================
           BOTÓN RESET
           Apaga completamente el RGB
           ===================================================== */
        if (button_pressed(button_rgb1.button_reset.gpio_num))
        {
            printf("Boton reset presionado\n");

            // Reinicia todos los porcentajes
            percentage_red = 0;
            percentage_green = 0;
            percentage_blue = 0;

            // Apaga LED
            set_led_rgb_percentage_given_values(
                &led_rgb1,
                0,
                0,
                0
            );
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}