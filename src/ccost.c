// ccost.c  08/08/2021  D.J.Whale

#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "port.h"
#include "timer.h"
#include "ser.h"
#include "spi.h"
#include "ism.h"

#define CC_TYPE_METER   0x00
#define CC_TYPE_COUNTER 0x04
#define CC_TYPE_PAIR    0x08

static const char METER[]   PROGMEM = "meter";
static const char PAIR[]    PROGMEM = "pair";
static const char COUNTER[] PROGMEM = "counter";
static const char UNKNOWN[] PROGMEM = "unknown";

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

    ser_hex(id1);
    ser_hex(id2);
    ser_tx(',');

    switch (type)
    {
        case CC_TYPE_METER:
            ser_txromstr(METER);
            show_watts(watt);
        break;

        case CC_TYPE_PAIR:
            ser_txromstr(PAIR);
            show_watts(watt);
        break;

        case CC_TYPE_COUNTER:
            ser_txromstr(COUNTER);
            ser_hexbuf(buf+2, 6);
        break;

        default:
            ser_txromstr(UNKNOWN);
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
    ism_init();
    ism_setmode(ISM_MODE_STBY);
    ism_setconfig(ISM_CONFIG_CC_FSK);
    ism_setmode(ISM_MODE_RX);
    sei(); // enable interrupts
}

//------------------------------------------------------------------------------
static void loop(void)
{
    uint8_t buf[8];
    if (ISM_RESULT_I_READY == ism_receive_waiting())
    {
        if (ISM_RESULT_OK == ism_rx(buf, sizeof(buf)))
        { // DECODE
            decode_payload(buf);
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
