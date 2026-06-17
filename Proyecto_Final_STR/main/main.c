/**
 * Application entry point.
 */

#include "nvs_flash.h"
#include "wifi_app.h"
#include "driver/gpio.h"
#include "ntc.h"
#include "fan.h"
#include "temperature_control.h"
#include "rgb_led.h"
#include "servo_motor.h"
#include "curtain_manager.h"
#define BLINK_GPIO 2

static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    // Inicializar SNTP
    init_obtain_time();

    // Configurar LED
    configure_led();

    // Inicializar NTC
    ntc_init();

    // Iniciar WiFi + Web Server
    wifi_app_start();
	
	// Inicializar ventilador
	fan_start();

	// Iniciar control de temperatura
	temperature_control_start();
	
    // Inicializar RGB LED
    rgb_led_init();

    // Inicializar servo motor
    servo_init();

    // Inicializar gestor de cortinas
    curtain_init();
    curtain_set_mode(1);

    curtain_set_manual_percent(50);
    
}
