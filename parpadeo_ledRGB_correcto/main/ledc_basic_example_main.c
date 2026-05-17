#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Pines del LED RGB
#define LED_R 2
#define LED_G 3
#define LED_B 4

void app_main(void)
{
    // Configurar pines
    gpio_reset_pin(LED_R);
    gpio_reset_pin(LED_G);
    gpio_reset_pin(LED_B);

    gpio_set_direction(LED_R, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_G, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);

    // Ánodo común:
    // 0 = ENCENDER
    // 1 = APAGAR

    while (1)
    {
        // ROJO
        gpio_set_level(LED_R, 0);
        gpio_set_level(LED_G, 1);
        gpio_set_level(LED_B, 1);

        printf("ROJO\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        // VERDE
        gpio_set_level(LED_R, 1);
        gpio_set_level(LED_G, 0);
        gpio_set_level(LED_B, 1);

        printf("VERDE\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        // AZUL
        gpio_set_level(LED_R, 1);
        gpio_set_level(LED_G, 1);
        gpio_set_level(LED_B, 0);

        printf("AZUL\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        // BLANCO
        gpio_set_level(LED_R, 0);
        gpio_set_level(LED_G, 0);
        gpio_set_level(LED_B, 0);

        printf("BLANCO\n");
        vTaskDelay(pdMS_TO_TICKS(1000));

        // APAGAR TODO
        gpio_set_level(LED_R, 1);
        gpio_set_level(LED_G, 1);
        gpio_set_level(LED_B, 1);

        printf("APAGADO\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}