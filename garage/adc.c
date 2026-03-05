#include <avr/io.h>
#include <util/delay.h>

#include "adc.h"

#define ADC0 0x00
#define ADC1 0x01
#define ADC2 0x02
#define ADC3 0x03
#define ADC4 0x04
#define ADC5 0x05
#define ADC6 0x06
#define ADC7 0x07
#define ADCVBG 0x0E
#define ADCGND 0x0F

void
adc_init(void)
{
    // Enable ADC, prescaler = 128 (16 MHz / 128 = 125 kHz)
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Disable digital input on ADC0
    // DIDR0 = (1 << ADC0D);
    ADMUX = (1 << REFS0);
}

static inline uint16_t
adc_read_channel(uint8_t ch)
{
    ADMUX = (ADMUX & 0xF0) | ch;
    _delay_us(10);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

uint16_t
adc_read_ch0()
{
    return adc_read_channel(ADC0);
}

uint16_t
adc_read_ch1()
{
    return adc_read_channel(ADC1);
}

uint16_t
adc_read_ch2()
{
    return adc_read_channel(ADC2);
}

uint16_t
adc_read_vbg()
{
    return adc_read_channel(ADCVBG);
}

uint16_t
adc_read_gnd()
{
    return adc_read_channel(ADCGND);
}