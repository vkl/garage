#ifndef _TIMER_1_H
#define _TIMER_1_H

#include <stdint.h>

void timer1_init(void);
void timer1_start(void);
void timer1_stop(void);
uint16_t timer1_get_distance(void);

#endif
