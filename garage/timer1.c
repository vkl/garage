#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

static volatile uint32_t t1_ovf;
static volatile uint16_t t1_start_cnt;
static volatile uint16_t t1_stop_cnt;
static volatile uint8_t is_ready = 0;

ISR(TIMER1_OVF_vect)
{
    t1_ovf++;
}

ISR(TIMER1_CAPT_vect)
{
    if (TCCR1B & (1 << ICES1)) {
        t1_start_cnt = ICR1;
        TCCR1B &= ~(1 << ICES1);
    } else {
        t1_stop_cnt = ICR1;
        TCCR1B |= (1 << ICES1);
        is_ready = 1;
    }
    
}

void
timer1_init(void)
{
    // Normal mode
    TCCR1A = 0;
    TCCR1B = (1 << ICNC1);
    TCNT1 = 0;
    t1_ovf = 0;
    // overflow irq, input capture
    TIMSK1 |= ((1 << ICIE1) | (1 << TOIE1));
}

static inline void 
timer1_start(void)
{
    // Prescaler 8, rising edge event
    TCCR1B |= (1 << CS11) | (1 << ICES1);
    is_ready = 0;
}

static inline void
timer1_stop(void)
{
    TCCR1B = 0;
    t1_ovf = 0;
    t1_start_cnt = 0;
    t1_stop_cnt = 0;
    is_ready = 0;
}

uint16_t
timer1_get_distance()
{
    uint16_t d = 0;
    timer1_start();
    while (is_ready == 0) _delay_ms(1);
    d = (((t1_ovf << 16) + t1_stop_cnt - t1_start_cnt) / 2) / 58;
    timer1_stop();
    return d;
}