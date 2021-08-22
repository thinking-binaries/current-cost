// rfm69.h  23/05/2021  D.J.Whale
// Interface to an Industrial/Scientific/Medical band radio (RFM69)

#if !defined(RFM69_H)
#define RFM69_H 1

#include <stdint.h>

typedef uint8_t RFM69_RESULT;
#define RFM69_RESULT_IS_ERROR(R) (((R)&0x80) == 0x80)
#define RFM69_RESULT_OK                  0x00
#define RFM69_RESULT_I_PAYLOAD           0x01
#define RFM69_RESULT_I_NOPAYLOAD         0x02
#define RFM69_RESULT_I_READY             0x03
#define RFM69_RESULT_I_NOTREADY          0x04

#define RFM69_RESULT_E_NORESPONSE        0x80
#define RFM69_RESULT_E_WRONGVER          0x81
#define RFM69_RESULT_E_BUFFER_TOO_SMALL  0x82
#define RFM69_RESULT_E_INVALID_PARAMETER 0x83
#define RFM69_RESULT_E_OVERRUN           0x84
#define RFM69_RESULT_E_LONG_PAYLOAD      0x85

#define RFM69_RESULT_E_UNIMPLEMENTED     0xFF


// maximum transfer unit (FIFO buffer limit)
// not strictly true if the FIFO is interleaved

#define RFM69_MTU 66

typedef uint8_t RFM69_MODE;
#define RFM69_MODE_STBY 0
#define RFM69_MODE_RX 1
#define RFM69_MODE_TX 2

typedef uint8_t RFM69_CONFIG;
//#define RFM69_CONFIG_ENG_OOK    0
//#define RFM69_CONFIG_ENG_FSK    1
#define RFM69_CONFIG_CC_FSK     2
#define RFM69_CONFIG_NONE       255

RFM69_RESULT rfm69_init(void);
uint8_t rfm69_radiover(void);

RFM69_RESULT rfm69_setmode(RFM69_MODE mode);
RFM69_RESULT rfm69_setconfig(RFM69_CONFIG config);
RFM69_CONFIG rfm69_getconfig(void);
RFM69_RESULT rfm69_tx(uint8_t * ppayload, uint8_t len, uint8_t times);

RFM69_RESULT rfm69_receive_waiting(void);
RFM69_RESULT rfm69_rxcbp(uint8_t * ppayload, uint8_t maxlen);
RFM69_RESULT rfm69_rx(uint8_t * ppayload, uint8_t maxlen);

#endif

// END: rfm69.h
