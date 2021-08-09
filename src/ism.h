// ism.h  23/05/2021  D.J.Whale
// Interface to an Industrial/Scientific/Medical band radio

#if !defined(ISM_H)
#define ISM_H 1

#include <stdint.h>

typedef uint8_t ISM_RESULT;
#define ISM_RESULT_IS_ERROR(R) (((R)&0x80) == 0x80)
#define ISM_RESULT_OK                  0x00
#define ISM_RESULT_I_PAYLOAD           0x01
#define ISM_RESULT_I_NOPAYLOAD         0x02
#define ISM_RESULT_I_READY             0x03
#define ISM_RESULT_I_NOTREADY          0x04

#define ISM_RESULT_E_NORESPONSE        0x80
#define ISM_RESULT_E_WRONGVER          0x81
#define ISM_RESULT_E_BUFFER_TOO_SMALL  0x82
#define ISM_RESULT_E_INVALID_PARAMETER 0x83
#define ISM_RESULT_E_OVERRUN           0x84
#define ISM_RESULT_E_LONG_PAYLOAD      0x85

#define ISM_RESULT_E_UNIMPLEMENTED     0xFF


// maximum transfer unit (FIFO buffer limit)
// not strictly true if the FIFO is interleaved

#define ISM_MTU 66

typedef uint8_t ISM_MODE;
#define ISM_MODE_STBY 0
#define ISM_MODE_RX 1
#define ISM_MODE_TX 2

typedef uint8_t ISM_CONFIG;
#define ISM_CONFIG_ENG_OOK    0
#define ISM_CONFIG_ENG_FSK    1
#define ISM_CONFIG_CC_FSK     2
#define ISM_CONFIG_NONE       255

ISM_RESULT ism_init(void);
uint8_t ism_radiover(void);

ISM_RESULT ism_setmode(ISM_MODE mode);
ISM_RESULT ism_setconfig(ISM_CONFIG config);
ISM_CONFIG ism_getconfig(void);
ISM_RESULT ism_tx(uint8_t * ppayload, uint8_t len, uint8_t times);

ISM_RESULT ism_receive_waiting(void);
ISM_RESULT ism_rxcbp(uint8_t * ppayload, uint8_t maxlen);
ISM_RESULT ism_rx(uint8_t * ppayload, uint8_t maxlen);

#endif

// END: ism.h
