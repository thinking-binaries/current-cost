// ser.h  17/05/2021  D.J.Whale
//
// Serial port

#ifndef _SER_H
#define _SER_H

//Define this to replace with a dummy serial port that does nothing
//#define SER_CFGDIS

#include "port.h"

typedef uint8_t SER_RESULT;
#define SER_RESULT_IS_ERROR(R)     (((R) & 0x80) == 0x80)

#define SER_RESULT_I_MISSED      0x01
#define SER_RESULT_I_NOTHING     0x02
#define SER_RESULT_I_NOISE       0x03
#define SER_RESULT_I_DATA        0x04
#define SER_RESULT_E_FRAMING     0x81
#define SER_RESULT_E_LINEBREAK   0x82


//9600bps - 1 bit is 26 DMX bits, gives 3.75 updates of 256ch per sec (4Hz limit)
#define SER_BITTIME_US      104
#define SER_HALF_BITTIME_US (SER_BITTIME_US/2)
//#define SER_BREAKTIME_US    (SER_BITTIME_US*9)  // 234

//38400bps, break would be 9 bits of low, 9*26=234
//NOTE:if the baud rate is slowed down at all, this multiplier will break a U8
//specifically for the break detector.
//but the baud rate would survive a U8 down to 4800bps.
//TODO: could do the break detector with an outer loop counter perhaps?
//#define SER_BITTIME_US      26
//#define SER_HALF_BITTIME_US (SER_BITTIME_US/2)
//#define SER_BREAKTIME_US    (SER_BITTIME_US*9)  // 234


/***** FUNCTION PROTOTYPES *****/

#if !defined(SER_CFGDIS)
void ser_as_tx(void);
void ser_as_rx(void);
void ser_tx(uint8_t data);
SER_RESULT ser_rx(uint8_t* pData);
uint8_t ser_wait(void);
void ser_txstr(char * str);
void ser_txromstr(const char * str);
bool ser_waitbrk(uint32_t for_us);
void ser_waitnobrk(void);
bool ser_is_low(void);
void ser_nl(void);
void ser_hex(uint8_t value);
void ser_u16(uint16_t v);
void ser_hexbuf(uint8_t * buf, uint8_t len);

#else

#define ser_as_tx()
#define ser_as_rx()
#define ser_tx(U8DATA)
#define ser_rx(PU8DATA) SER_RESULT_I_NOTHING
#define ser_wait() 0
#define ser_txstr(PCHAR)
bool ser_waitfor(uint32_t for_us);
#define ser_waitbrk(U32FOR) ser_waitfor(U32FOR)
#define ser_waitnobrk()
#define ser_is_low() false
#define ser_nl()
#define ser_hex(U8V)
#define ser_hexbuf(PU8BUF, U8LEN)

#endif

#endif