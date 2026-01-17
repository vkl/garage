#include <avr/io.h>

#include "adc.h"

void
adc_init(void)
{
    // Enable ADC, prescaler = 128 (16 MHz / 128 = 125 kHz)
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Disable digital input on ADC0
    // DIDR0 = (1 << ADC0D);
}

uint16_t
adc_read_test()
{
    // Reference = AVcc, channel = internal 1.1
    ADMUX = (1 << REFS0) | 0x0E;
    ADCSRA |= (1 << ADSC);               // start conversion
    while (ADCSRA & (1 << ADSC));        // wait

    return ADC;  // reads ADCL then ADCH
}


uint16_t
adc_read(uint8_t ch)
{
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);  // select channel

    ADCSRA |= (1 << ADSC);               // start conversion
    while (ADCSRA & (1 << ADSC));        // wait

    return ADC;  // reads ADCL then ADCH
}
