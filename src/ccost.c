// ccost.c  08/08/2021  D.J.Whale

#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "port.h"
#include "timer.h"
#include "ser.h"
#include "spi.h"
#include "rfm69.h"

#define CC_PAYLOAD_SIZE_MANCH 16
#define CC_TYPE_METER         0x00
#define CC_TYPE_COUNTER       0x04
#define CC_TYPE_PAIR          0x08

#define DEBUG 1

static const char STR_RAW[]           PROGMEM = "RAW:";
static const char STR_ERR_BAD_MANCH[] PROGMEM = "BADM:";
static const char STR_OK_BUF[]        PROGMEM = "OK:";
static const char STR_DATA[]          PROGMEM = "DATA:";
static const char STR_METER[]         PROGMEM = "meter";
static const char STR_PAIR[]          PROGMEM = "pair";
static const char STR_COUNTER[]       PROGMEM = "counter";
static const char STR_UNKNOWN[]       PROGMEM = "unknown";

//------------------------------------------------------------------------------
static void show_watts(uint16_t watt[3])
{
    ser_tx(',');
    for (uint8_t i=0; i<3; i++)
    {
        if (watt[i] & 0x8000) // valid if high bit set
        {
            ser_u16(watt[i] & 0x7FFF);
        }
        if (i != 2) ser_tx(',');
    }
}

//------------------------------------------------------------------------------
static void decode_payload(uint8_t * buf)
{
    // nybbles, in 8 bytes
    // type:id id:id w1:w1 w1:w1 w2:w2 w2:w2 w3:w3 w3:w3
    uint8_t type   = (buf[0] & 0xF0)>>4;
    uint8_t id1    = buf[0] & 0x0F;
    uint8_t id2    = buf[1];
    uint16_t watt[3];
    watt[0]        = (((uint16_t)buf[2])<<8) | buf[3]; // big-endian
    watt[1]        = (((uint16_t)buf[4])<<8) | buf[5]; // big-endian
    watt[2]        = (((uint16_t)buf[6])<<8) | buf[7]; // big-endian

    ser_txromstr(STR_DATA);
    ser_hex(id1);
    ser_hex(id2);
    ser_tx(',');

    switch (type)
    {
        case CC_TYPE_METER:
            ser_txromstr(STR_METER);
            show_watts(watt);
        break;

        case CC_TYPE_PAIR:
            ser_txromstr(STR_PAIR);
            show_watts(watt);
        break;

        case CC_TYPE_COUNTER:
            ser_txromstr(STR_COUNTER);
            ser_tx(',');
            ser_hexbuf(buf+2, 6); //TODO: decode as a U24?
        break;

        default:
            ser_txromstr(STR_UNKNOWN);
            ser_tx(',');
            ser_hexbuf(buf, 8);
        break;
    }
    ser_nl();
}

//------------------------------------------------------------------------------
static void setup(void)
{
    timer_start();   // 1uS tick service
    ser_as_tx();
    spi_init(SPI_CPOL0|SPI_CPHA0|SPI_CSPOL1);
    rfm69_init();
    rfm69_setmode(RFM69_MODE_STBY);
    rfm69_setconfig(RFM69_CONFIG_CC_FSK);
    rfm69_setmode(RFM69_MODE_RX);
    sei(); // enable interrupts
}

//------------------------------------------------------------------------------
static bool manch_is_valid(uint8_t * buf, uint8_t size)
{
    // validate manchester bits as 5,6,9 or A (01 01, 01 10, 10 01, 10 10)
    for (uint8_t i=0; i<sizeof(buf); i++)
    {
        // high nybble
        uint8_t n = buf[i]>>4;
        if ((n != 0x05) && (n != 0x06) && (n != 0x09) && (n != 0x0A))
        {
            return false; // INVALID, early bail
        }
        // low nybble
        n = buf[i] & 0x0F;
        if ((n != 0x05) && (n != 0x06) && (n != 0x09) && (n != 0x0A))
        {
            return false; // INVALID, early bail
        }
    }
    return true; // VALID
}

//------------------------------------------------------------------------------
// produce a whole byte of output (8 bits) from 2 byes (16 bits)
// first bit of a valid pair of bits is the actual data bit
// 5 = 01 01   (00)
// 6 = 01 10   (01)
// 9 = 10 01   (10)
// A = 10 10   (11)
// each input byte (8 bits) produces 1 nybble of data bits (4 bits)

static uint8_t manch_extract(uint8_t * buf)
{
    uint8_t in;
    uint8_t out;

    // extract 4 bits from even byte A.B.C.D. into high nybble ABCD....
    in  = buf[0];
    out = (in & 0x80) | ((in & 0x20)<<1) | ((in & 0x08)<<2) | ((in & 0x02)<<3);

    // extract 4 bits from odd byte E.F.G.H. into low nybble   ....EFGH
    in = buf[1];
    out |= ((in & 0x80)>>4) | ((in & 0x20)>>3) | ((in & 0x08)>>2) | ((in & 0x02) >> 1);

    return out;
}

//------------------------------------------------------------------------------
// decode manchester bits in-place into first half of buf

static void manch_decode(uint8_t * buf, uint8_t size)
{
    for (uint8_t w=0; w<size/2; w++)
    {
        buf[w] = manch_extract(buf+(w*2));
    }
}

//------------------------------------------------------------------------------
static void loop(void)
{
    uint8_t buf[CC_PAYLOAD_SIZE_MANCH];
    if (RFM69_RESULT_I_READY == rfm69_receive_waiting())
    {
        if (RFM69_RESULT_OK == rfm69_rx(buf, sizeof(buf)))
        { // DECODE
#if defined(DEBUG)
            ser_txromstr(STR_RAW);
            ser_hexbuf(buf, sizeof(buf));
            ser_nl();
#endif

            if (! manch_is_valid(buf, sizeof(buf)))
            {
#if defined(DEBUG)
                ser_txromstr(STR_ERR_BAD_MANCH);
                ser_hexbuf(buf, sizeof(buf));
                ser_nl();
#endif
            }
            else // valid
            {
                // decode manchester bits in-place into first half of buf
                manch_decode(buf, sizeof(buf));

#if defined(DEBUG)
                // dump new payload
                ser_txromstr(STR_OK_BUF);
                ser_hexbuf(buf, sizeof(buf)/2);
                ser_nl();
#endif
                decode_payload(buf); // friendly CSV decode
            }
        }
    }
}

//------------------------------------------------------------------------------
int main(void)
{
    setup();
    while (true)
    {
        loop();
    }
    return 0;
}


//END: ccost.c
