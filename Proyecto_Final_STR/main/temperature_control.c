#include "temperature_control.h"
#include "ntc.h"
#include "fan.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

float desired_temperature = 20.0;
float maximum_temperature = 35.0;

static void temperature_task(void *pv)
{
    while(1)
    {
        float temp = ntc_get_temperature();

        fan_update_auto(
            temp,
            desired_temperature,
            maximum_temperature
        );

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void temperature_control_start(void)
{
    xTaskCreate(
        temperature_task,
        "temperature_task",
        4096,
        NULL,
        5,
        NULL
    );
}