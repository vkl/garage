#ifndef _WS2812_H
#define _WS2812_H

#include <zephyr/drivers/led_strip.h>

#include <nrfx_pwm.h>

#define STRIP_NUM_PIXELS    32

typedef void (*ws2812_update_cb_t)(struct led_rgb *pixels, size_t count);

int ws2812_pwm_init(void);
void ws2812_clear(void);
void ws2812_pwm_update(ws2812_update_cb_t update_cb);

#endif /* _WS2812_H */