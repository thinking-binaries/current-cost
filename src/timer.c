// timer.c  17/05/2021  D.J.Whale
//
// Timer and delay services

#include "timer.h"

//-----------------------------------------------------------------------------
void timer_start(void)
{
  /* CONFIGURE TIMER 1 */
  // CS1[3210] = 0100 8MHz/8 = 1MHz = 1uS ticks
  TCCR1 |= (1<<CS12);
}

//-----------------------------------------------------------------------------
uint8_t timer_diff(uint8_t earlier, uint8_t later)
{
  if (earlier <= later)
  { /* no wrap */
    return later - earlier;
  }
  else
  { /* has wrapped */
    return (255-earlier) + 1 + later;
  }
}


//-----------------------------------------------------------------------------
void timer_wait_until(uint8_t target)
{
  if (target <= timer_read())
  {
    while (timer_read() > target)
    {
      /* busy wait for modulo256 wrap */
    }
  }

  while (timer_read() < target)
  {
    /* busy wait for target reached */
  }
}


//-----------------------------------------------------------------------------
void timer_delay_us(uint8_t amount)
{
  /* allowable modulo256 wrap */
  timer_wait_until(timer_read() + amount); // tail call optimised
}


//-----------------------------------------------------------------------------
void timer_delay_ms(uint8_t amount) // up to 255ish ms
{
  uint8_t t = timer_read();

  while (amount > 0)
  {
    // Delay 1ms (200uS * 5 = 1000us)
    uint8_t inner;
    for (inner=0; inner<5; inner++)
    {
      t += 200; // Modulo256 wrap is intentional
      timer_wait_until(t);
    }
    amount--;
  }
}

// END OF FILE
