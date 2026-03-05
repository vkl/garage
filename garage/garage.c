/*
 * garage.c
 *
 * Created: 11/15/2025 10:42:37 AM
 * Author : vklad
 */ 
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "log.h" 
#include "ws2812.h"
#include "timer1.h"
#include "timer2.h"
#include "adc.h"

#ifdef TRACE
#include "usart.h"
#endif

#define TRIG_DDR  DDRD
#define TRIG_PORT PORTD
#define TRIG_PIN  PORTD3

#define ECHO_DDR  DDRB
#define ECHO_PORT PORTB
#define ECHO_PINR PINB
#define ECHO_PIN  PINB0

static inline void
display(const uint16_t d, const uint16_t low_thrs,
    const uint16_t high_thrs)
{
    uint8_t i, max;

    if (d >= high_thrs) {
        max = 0;
    } else if (d < low_thrs) {
        max = 16;
    } else {
        uint32_t tmp = (uint32_t)PIXELS * (high_thrs - d);
        max = (uint8_t)(tmp / (high_thrs - low_thrs));
    }

    for (i = 0; i < PIXELS; i++) {
        if (i < max) {
            if (i < 6)
                ws2812_setPixel(GREEN, i);
            else if (i < 12)
                ws2812_setPixel(YELLOW, i);
            else
                ws2812_setPixel(RED, i);
        } else {
            ws2812_setPixel(OFF, i);
        }
    }

    LOG("distance: %u, max: %u, low: %u, high: %u\n",
            d, max, low_thrs, high_thrs);

    ws2812_refresh();

}

static inline void
trig_10us(void)
{
    TRIG_PORT &= ~(1<<TRIG_PIN);
    _delay_us(2);

    TRIG_PORT |= (1<<TRIG_PIN);
    _delay_us(10);
    TRIG_PORT &= ~(1<<TRIG_PIN);
}

int
main(void)
{
    uint16_t d = 0;
    uint16_t low_thrs;
    uint16_t high_thrs;
    cli();
    ws2812_timer_init();
    timer1_init();
    adc_init();
#ifdef TRACE
    USART_Init();
#endif
    sei();

    ECHO_DDR &= ~(1 << ECHO_PIN);
    ECHO_PORT &= ~(1 << ECHO_PIN);

    TRIG_DDR |= (1 << TRIG_PIN);

    for (;;) {
        low_thrs = adc_read_ch0();
        high_thrs = adc_read_ch1();
        trig_10us();
        d = timer1_get_distance();
        display(d, low_thrs, high_thrs);
        _delay_ms(700);
    }

    return 0;
}

