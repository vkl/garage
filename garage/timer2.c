#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer2.h"

static volatile uint32_t t2_ovf;

ISR(TIMER2_OVF_vect)
{
    t2_ovf++;
}

uint16_t
timer2_get_distance(void)
{
    uint32_t t    = (t2_ovf << 8) | TCNT2;
    uint32_t us   = t / 2;
    // HC-SR04: cm ? us / 58
    return (uint16_t)(us / 58);
}

void
timer2_init(void)
{
    // Normal mode
    TCCR2A = 0;
    TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));

    TCNT2 = 0;
    t2_ovf = 0;
    TIMSK2 = (1 << TOIE2); // overflow irq
}

void
timer2_start(void)
{
    /* Pre scaler /8: tick = 0.5 us @ 16MHz */
    TCCR2B = (1 << CS21);
    TCNT2 = 0;
    t2_ovf = 0;
}

void
timer2_stop(void)
{
    TCCR2B = 0;
}
