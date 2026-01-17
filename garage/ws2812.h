/*
 * ws2812.h
 *
 * Created: 1/2/2026 10:00:18 PM
 *  Author: vklad
 */ 


#ifndef WS2812_H_
#define WS2812_H_

#include <stdint.h>

#define OFF     0x000000
#define CYAN    0xFF00FF
#define BLUE    0x0000FF
#define ORANGE  0x80FF00
#define YELLOW  0xFFFF00
#define GREEN   0xFF0000
#define PURPLE  0x20B0F0
#define RED     0x00FF00
#define WEAKGREEN 0x770000

#define PERIOD 20 /* Timer 0 OCR0A */
#define BIT_HIGH 14
#define BIT_LOW 6

#define RESET_DELAY 200 /* delay us */

#define PIXELS 16
#define RAINBOW_COLORS 12

void ws2812_timer_init(void);
void ws2812_refresh(void);
void ws2812_clear(void);
void ws2812_fillColor(uint32_t color);
void ws2812_setPixel(uint32_t color, uint8_t i);

#endif /* WS2812_H_ */