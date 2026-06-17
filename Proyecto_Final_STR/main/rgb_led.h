#ifndef RGB_LED_H_
#define RGB_LED_H_

#include <stdint.h>

void rgb_led_init(void);

void rgb_led_set_color(
    uint8_t red,
    uint8_t green,
    uint8_t blue);

void rgb_led_set_brightness(uint8_t percent);

void rgb_led_wifi_app_started(void);
void rgb_led_wifi_connected(void);
void rgb_led_http_server_started(void);
#endif