#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>

void adc_init(void);
uint16_t adc_read_ch0(void);
uint16_t adc_read_ch1(void);
uint16_t adc_read_ch2(void);
uint16_t adc_read_vbg(void);
uint16_t adc_read_gnd(void);

#endif
