/*
 * ESP-IDF ADC Example (ESP32-C6)
 * --------------------------------
 * Este ejemplo lee 2 canales ADC usando ADC1.
 * Incluye:
 * - Lectura RAW
 * - Conversión a voltaje (calibración)
 * - ADC2 opcional (desactivado por defecto)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

const static char *TAG = "EXAMPLE";

/*---------------------------------------------------------------
        CONFIGURACIÓN ADC
---------------------------------------------------------------*/

// Canales ADC1 (depende del chip)
// En ESP32-C6 los canales se asignan a GPIOs específicos
#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_2   // Ej: GPIO2
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_3   // Ej: GPIO3

#define EXAMPLE_ADC_ATTEN           ADC_ATTEN_DB_12 // rango hasta ~3.3V

/*---------------------------------------------------------------
        ADC2 (DESACTIVADO)
---------------------------------------------------------------*/
/*
 * ADC2 no se está utilizando en este proyecto.
 * Se deja incluido del ejemplo original por compatibilidad.
 * En ESP32-C3/C6 normalmente no es necesario usarlo.
 */

static int adc_raw[2][10];     // almacena lecturas RAW
static int voltage[2][10];     // almacena voltaje convertido

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel,
                                         adc_atten_t atten, adc_cali_handle_t *out_handle);

static void example_adc_calibration_deinit(adc_cali_handle_t handle);

void app_main(void)
{
    /*-------------------------------------------------------
            INICIALIZACIÓN ADC1
    -------------------------------------------------------*/
    adc_oneshot_unit_handle_t adc1_handle;

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    /*-------------------------------------------------------
            CONFIGURACIÓN DE CANALES
    -------------------------------------------------------*/
    adc_oneshot_chan_cfg_t config = {
        .atten = EXAMPLE_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

    /*-------------------------------------------------------
            CALIBRACIÓN ADC
    -------------------------------------------------------*/
    adc_cali_handle_t adc1_cali_chan0_handle = NULL;
    adc_cali_handle_t adc1_cali_chan1_handle = NULL;

    bool do_calibration1_chan0 =
        example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0,
                                     EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);

    bool do_calibration1_chan1 =
        example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN1,
                                     EXAMPLE_ADC_ATTEN, &adc1_cali_chan1_handle);

    /*-------------------------------------------------------
            LOOP PRINCIPAL
    -------------------------------------------------------*/
    while (1) {

        /* -------- LECTURA CANAL 0 -------- */
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0][0]));
        ESP_LOGI(TAG, "ADC CH0 RAW: %d", adc_raw[0][0]);

        if (do_calibration1_chan0) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
                adc1_cali_chan0_handle,
                adc_raw[0][0],
                &voltage[0][0]
            ));
            ESP_LOGI(TAG, "CH0 VOLTAGE: %d mV", voltage[0][0]);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        /* -------- LECTURA CANAL 1 -------- */
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_raw[0][1]));
        ESP_LOGI(TAG, "ADC CH1 RAW: %d", adc_raw[0][1]);

        if (do_calibration1_chan1) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
                adc1_cali_chan1_handle,
                adc_raw[0][1],
                &voltage[0][1]
            ));
            ESP_LOGI(TAG, "CH1 VOLTAGE: %d mV", voltage[0][1]);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        /*
         * ADC2:
         * Este bloque está desactivado porque este proyecto solo usa ADC1.
         * Se mantiene del ejemplo original por referencia.
         */
    }

    /*-------------------------------------------------------
            CLEANUP (no se ejecuta por loop infinito)
    -------------------------------------------------------*/
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));

    if (do_calibration1_chan0) {
        example_adc_calibration_deinit(adc1_cali_chan0_handle);
    }

    if (do_calibration1_chan1) {
        example_adc_calibration_deinit(adc1_cali_chan1_handle);
    }
}

/*---------------------------------------------------------------
        CALIBRACIÓN ADC
---------------------------------------------------------------*/

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel,
                                         adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    {
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) calibrated = true;
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) calibrated = true;
    }
#endif

    *out_handle = handle;

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration OK");
    } else {
        ESP_LOGW(TAG, "Calibration not available (still works with RAW)");
    }

    return calibrated;
}

static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_delete_scheme_curve_fitting(handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_delete_scheme_line_fitting(handle);
#endif
}