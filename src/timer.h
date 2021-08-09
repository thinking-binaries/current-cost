// timer.h  17/05/2021  D.J.Whale
//
// Timer and delay services
// works in 1uS ticks internally, max range 256 (U8)

#ifndef _TIMER_H
#define _TIMER_H

#include "port.h"

void timer_start(void);
#define timer_read() TCNT1
#define timer_write(V) TCNT1 = V

uint8_t timer_diff(uint8_t earlier, uint8_t later);
void timer_wait_until(uint8_t target);
void timer_delay_us(uint8_t amount);
void timer_delay_ms(uint8_t amount);

#endif

