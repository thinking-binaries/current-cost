// ser.c  17/05/2021  D.J.Whale
//
// Half-duplex serial port

#include <stdbool.h>
#include <stdint.h>

#include <avr/pgmspace.h>

#include "ser.h"
#include "timer.h"
#include "port.h"


//===== TRANSMITTER ============================================================

//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_as_tx(void)
{
  SER_AS_OUT();
  SER_HIGH();
}
#endif


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_tx(uint8_t data)
{
  uint8_t i;
  uint8_t t;

  //START bit
  t = timer_read();
  SER_LOW();

  //LSB first data
  for (i=0; i<8; i++)
  {
    t += SER_BITTIME_US;
    if (data & 0x01)
    {
      timer_wait_until(t);
      SER_HIGH();
    }
    else
    {
      timer_wait_until(t);
      SER_LOW();
    }
    data >>= 1; // LSB first
  }

  // delay for last data bit
  t += SER_BITTIME_US;
  timer_wait_until(t);

  //2 STOP bits to give receiver uart time to process data
  t += (SER_BITTIME_US + SER_BITTIME_US);
  SER_HIGH();
  timer_wait_until(t);
}
#endif


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_txstr(char * str)
{
    char ch;

    while ((ch = *str++))
    {
        ser_tx(ch);
    }
}
#endif


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_txromstr(const char * str)
{
    char ch;

    while (0 != (ch = pgm_read_byte_near(str++)))
    {
        ser_tx(ch);
    }
}
#endif


//===== RECEIVER ===============================================================

//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_as_rx(void)
{
  SER_AS_IN();
  SER_LOW(); // pullups off
}
#endif


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
SER_RESULT ser_rx(uint8_t* pData)
{
  uint8_t t;
  uint8_t data;
  uint8_t bitcount;

  /* Delay half a bit to check middle of start bit */ //<<NOTE: might be anywhere in the start bit
  timer_write(0);
  t = SER_HALF_BITTIME_US;
  timer_wait_until(t);

  if (SER_IS_HIGH())
  { /* must just be a glitch */
    return SER_RESULT_I_NOISE;
  }

  /* shift 8 bits through the data register first */
  data = 0;
  bitcount = 1; // count the start bit in total
  while (bitcount < 9)
  {
    data >>= 1; // RX LSB first
    t += SER_BITTIME_US;
    timer_wait_until(t); // now at centre of next possible bit
    if (SER_READ())
    {
      data |= 0x80; // RX LSB first
    }
    bitcount++;
  }

  /* See if this looks like a valid data byte by checking stop bit */

  t += SER_BITTIME_US;
  timer_wait_until(t); // centre of first potential stop bit
  if (SER_IS_HIGH()) // 1 stop bit
  {
    *pData = data;
    return SER_RESULT_I_DATA;
  }

  /* must be a framing error or line break */
  if (0 == data)
  {
    return SER_RESULT_E_LINEBREAK;
  }
  return SER_RESULT_E_FRAMING;
}
#endif


//-----------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
uint8_t ser_wait(void)
{
    while (true)
    {
        uint8_t data;

        // wait for falling edge of possible start bit
        while (SER_IS_HIGH()) {}

        // try to decode what is there
        if (ser_rx(&data) == SER_RESULT_I_DATA)
        {
            return data;
        }
    }
}
#endif


//-----------------------------------------------------------------------------

#define BREAK_SAMPLE_US (SER_BITTIME_US/8)  // 13uS is 8 times oversampled at 9600bps (104uS per bit)
#define BREAK_LIMIT     ((SER_BITTIME_US*10) / BREAK_SAMPLE_US)  // 80*13 = 1040uS = 10 bits

#if !defined(SER_CFGDIS)
bool ser_waitbrk(uint32_t for_us)
{
    ser_as_rx();
    uint8_t t = timer_read();
    uint8_t low_count = 0;

    while (true) // loops 13us at a time
    {
        if (for_us <= BREAK_SAMPLE_US) {break;} // end of timeout

        for_us -= BREAK_SAMPLE_US;
        t += BREAK_SAMPLE_US; // intentional wrap at 255->0
        timer_wait_until(t); // timing gate

        if (SER_IS_LOW())
        {
            if (++low_count >= BREAK_LIMIT) {return true;} // BREAK CONDITION >= 1040uS (10 bits)
        }
        else
        {
            low_count = 0;
        }
    }
    return false; // normal timeout
}
#endif


//-----------------------------------------------------------------------------
bool ser_waitfor(uint32_t for_us)
{
    uint8_t t = timer_read();

    while (true) // loops 13us at a time
    {
        if (for_us <= BREAK_SAMPLE_US) {break;} // end of timeout

        for_us -= BREAK_SAMPLE_US;
        t += BREAK_SAMPLE_US; // intentional wrap at 255->0
        timer_wait_until(t); // timing gate
    }
    return false; // did not break
}


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_waitnobrk(void)
{
    ser_as_rx();
    while (SER_IS_LOW())
    {
    }
}
#endif


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
bool ser_is_low(void)
{
    return SER_IS_LOW();
}
#endif


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_nl(void)
{
    ser_tx('\r');
    ser_tx('\n');
}
#endif

//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_u16(uint16_t v)
{
    if (v == 0)
    {
        ser_tx('0');
        return;
    }

    bool zsuppress = true;
    uint16_t m = 10000;

    while (m != 0)
    {
        uint8_t d = v/m;

        if (d)
        {
            ser_tx('0' + d);
            zsuppress = false;
        }
        else
        {
            if (! zsuppress)
            {
                ser_tx('0' + d);
            }
        }
        v -=  (m * d);
        m /= 10;
    }
}
#endif

//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_hex(uint8_t value)
{
    for (uint8_t i=0; i<2; i++)
    {
        uint8_t nibble = (value & 0xF0)>>4;
        if (nibble < 0x0A) {ser_tx('0' + nibble);}
        else               {ser_tx('A' + nibble - 10);}
        value <<= 4;
    }
}
#endif


//------------------------------------------------------------------------------
#if !defined(SER_CFGDIS)
void ser_hexbuf(uint8_t * buf, uint8_t len)
{
    for (uint8_t i=0; i<len; i++)
    {
        ser_hex(buf[i]);
        ser_tx(' ');
    }
}
#endif


// END OF FILE
