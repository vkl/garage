/*
 * ws2812.c
 *
 * Created: 1/2/2026 10:00:03 PM
 *  Author: vklad
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

#include "ws2812.h"

volatile static uint8_t count;
volatile static uint8_t curr_pixel;

static uint32_t leds[PIXELS] = {0};

const uint32_t color[8] = {
    OFF, CYAN, BLUE, ORANGE, YELLOW, GREEN, PURPLE, RED
};

const uint32_t rainbow[] = {
    0xFF7F00, 0xFF0000, 0xFF007F, 0xFF00FF,
    0x7F00FF, 0x0000FF, 0x007FFF, 0x00FFFF,
    0x00FF7F, 0x00FF00, 0x7FFF00, 0xFFFF00
};

ISR(TIMER0_OVF_vect) {
    if (count > 23) {
        curr_pixel++;
        count = 0;
    }
    OCR0B = ((uint32_t)1 << (23 - count)) & leds[curr_pixel] ? BIT_HIGH : BIT_LOW;
    count++;
    if ((count >= 23) && (curr_pixel >= (PIXELS))) {
        TCCR0B = 0;
        _delay_us(RESET_DELAY);
    }
}

void
ws2812_timer_init(void)
{
    DDRD |= _BV(DDD5); // ws2812b pin data drive
    
    /* Timer0 Config */
    TCCR0A = 0x00;
    TCCR0B = 0x00;
    TIMSK0 |= _BV(TOIE0); // Timer/Counter0 Overflow Interrupt Enable
    TCCR0A |= (_BV(WGM00) | _BV(WGM01) | _BV(WGM02)); // Fast PWM Mode 7
    TCCR0A |= _BV(COM0B1); // Non-inverting mode
    OCR0A = PERIOD;
    TCNT0 = 0;
    /* Timer0 Config End */
}

void
ws2812_refresh(void)
{
    curr_pixel = 0;
    count = 0;
    OCR0B = ((uint32_t)1 << 23) & leds[curr_pixel] ? BIT_HIGH : BIT_LOW;
    TCCR0B |= (1 << CS00);
    count++;
    while (TCCR0B != 0) _delay_ms(1);
}

void
ws2812_clear(void)
{
    ws2812_fillColor(OFF);
}

void
ws2812_fillColor(uint32_t color)
{
    uint8_t p = 0;
    do {
        leds[p] = color;
    } while(++p < PIXELS);
}

void
ws2812_setPixel(uint32_t color, uint8_t i)
{
	if (i >= PIXELS) return;
	leds[i] = color;
}
