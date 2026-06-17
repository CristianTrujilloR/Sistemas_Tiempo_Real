#include "ntc.h"
#include <stdio.h>
#include <math.h>

#include "driver/adc.h"


#define NTC_GPIO_ADC ADC1_CHANNEL_6      // GPIO34

#define R_FIXED       100000.0f
#define R0            100000.0f
#define BETA          4190.0f
#define T0            298.15f

void ntc_init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(
        NTC_GPIO_ADC,
        ADC_ATTEN_DB_12);

}

float ntc_get_temperature(void)
{
    uint32_t adc_reading = 0;

    for(int i = 0; i < 64; i++)
    {
        adc_reading += adc1_get_raw(NTC_GPIO_ADC);
    }

    adc_reading /= 64;

    float v = ((float)adc_reading / 4095.0f) * 3.3f;

    if(v <= 0.01f)
        return -99.0f;

    float r_ntc =
        R_FIXED * (3.3f - v) / v;

    float tempK =
        1.0f /
        (
            (1.0f / T0)
            +
            (1.0f / BETA)
            *
            logf(r_ntc / R0)
        );

    printf("ADC=%ld  V=%.3f  RNTC=%.0f  TEMP=%.2f\n",
           adc_reading,
           v,
           r_ntc,
           tempK - 273.15f);

    return tempK - 273.15f;
}