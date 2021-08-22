// rfm69.c  23/05/2021  D.J.Whale
// Interface to an Industrial/Scientific/Medical band radio
// based on:  hrf69.h  03/04/2016  D.J.Whale

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "rfm69.h"
#include "spi.h"
#include "ser.h"

// CONFIG
#define DEFAULT_TIMES 2


//===== PRIVATE ================================================================

#define EXPECTED_RADIOVER 0x24
#define MAX_FIFO_BUFFER   66

typedef struct
{
    RFM69_MODE current_mode;
    RFM69_CONFIG config;
    uint8_t radiover;
} RFM69_DATA;

static RFM69_DATA _rfm69_data = {RFM69_MODE_STBY, RFM69_CONFIG_NONE, 0};


typedef struct
{
  uint8_t addr;
  uint8_t value;
} RFM69_CONFIG_REC;


// Register addresses - high bit clear for read, set for write.
// Precise register descriptions can be found in:
// www.hoperf.com/upload/rf/RFM69W-V1.3.pdf on page 63-74

#define HRF_ADDR_FIFO                   0x00
#define HRF_ADDR_OPMODE                 0x01
#define HRF_ADDR_DATAMODUL              0x02
#define HRF_ADDR_BITRATEMSB             0x03
#define HRF_ADDR_BITRATELSB             0x04
#define HRF_ADDR_FDEVMSB                0x05
#define HRF_ADDR_FDEVLSB                0x06
#define HRF_ADDR_FRMSB                  0x07
#define HRF_ADDR_FRMID                  0x08
#define HRF_ADDR_FRLSB                  0x09
#define HRF_ADDR_OSC1                   0x0A
#define HRF_ADDR_AFCCTRL                0x0B
// RESERVED0C
#define HRF_ADDR_LISTEN1                0x0D
#define HRF_ADDR_LISTEN2                0x0E
#define HRF_ADDR_LISTEN3                0x0F
#define HRF_ADDR_VERSION                0x10
#define HRF_ADDR_PALEVEL                0x11
#define HRF_ADDR_PARAMP                 0x12
#define HRF_ADDR_OCP                    0x13
//RESERVED14
//RESERVED15
//RESERVED16
//RESERVED17
#define HRF_ADDR_LNA                    0x18
#define HRF_ADDR_RXBW                   0x19
#define HRF_ADDR_AFCBW                  0x1A
#define HRF_ADDR_OOKPEAK                0x1B
#define HRF_ADDR_OOKAVG                 0x1C
#define HRF_ADDR_OOKFIX                 0x1D
#define HRF_ADDR_AFCFEI                 0x1E
#define HRF_ADDR_AFCMSB                 0x1F
#define HRF_ADDR_AFCLSB                 0x20
#define HRF_ADDR_FE1MSB                 0x21
#define HRF_ADDR_FEILSB                 0x22
#define HRF_ADDR_RSSICONFIG             0x23
#define HRF_ADDR_RSSIVALUE              0x24
#define HRF_ADDR_DIOMAPPING1            0x25
#define HRF_ADDR_DIOMAPPING2            0x26
#define HRF_ADDR_IRQFLAGS1              0x27
#define HRF_ADDR_IRQFLAGS2              0x28
#define HRF_ADDR_RSSITHRESH             0x29
#define HRF_ADDR_RXTIMEOUT1             0x2A
#define HRF_ADDR_RXTIMEOUT2             0x2B
#define HRF_ADDR_PREAMBLEMSB            0x2C
#define HRF_ADDR_PREAMBLELSB            0x2D
#define HRF_ADDR_SYNCCONFIG             0x2E
#define HRF_ADDR_SYNCVALUE1             0x2F
#define HRF_ADDR_SYNCVALUE2             0x30
#define HRF_ADDR_SYNCVALUE3             0x31
#define HRF_ADDR_SYNCVALUE4             0x32
#define HRF_ADDR_SYNCVALUE5             0x33
#define HRF_ADDR_SYNCVALUE6             0x34
#define HRF_ADDR_SYNCVALUE7             0x35
#define HRF_ADDR_SYNCVALUE8             0x36
#define HRF_ADDR_PACKETCONFIG1          0x37
#define HRF_ADDR_PAYLOADLEN             0x38
#define HRF_ADDR_NODEADRS               0x39
#define HRF_ADDR_BROADCASTADRS          0x3A
#define HRF_ADDR_AUTOMODES              0x3B
#define HRF_ADDR_FIFOTHRESH             0x3C
#define HRF_ADDR_PACKETCONFIG2          0x3D
#define HRF_ADDR_AESKEY1                0x3E
//AESKEY2..AESKEY16     3F..4D
#define HRF_ADDR_TEMP1                  0x4E
#define HRF_ADDR_TEMP2                  0x4F
//RESERVED50
//RESERVED51
//RESERVED52
//RESERVED53
//RESERVED54
//RESERVED55
//RESERVED56
//RESERVED57
#define HRF_ADDR_TESTLNA                0x58
//RESERVED59
#define HRF_ADDR_TESTPA1                0x5A
//RESERVED5B
#define HRF_ADDR_TESTPA2                0x5C
//RESERVED5D
//RESERVED5E
//RESERVED5F
//RESERVED60
//RESERVED61
//RESERVED62
//RESERVED63
//RESERVED64
//RESERVED65
//RESERVED66
//RESERVED67
//RESERVED68
//RESERVED69
//RESERVED6A
//RESERVED6B
//RESERVED6C
//RESERVED6D
//RESERVED6E
#define HRF_ADDR_TESTDAGC               0x6F
//RESERVED70
#define HRF_ADDR_TESTAFC                0x71
//RESERVED72
//RESERVED73
//RESERVED74
//RESERVED75
//RESERVED76
//RESERVED77
//RESERVED78
//RESERVED79
//RESERVED7A
//RESERVED7B
//RESERVED7C
//RESERVED7D
//RESERVED7E
//RESERVED7F

// Masks to set and clear bits
#define HRF_MASK_DATAMODUL_OOK         0x08
#define HRF_MASK_DATAMODUL_FSK         0x00
#define HRF_MASK_WRITE_DATA            0x80
#define HRF_MASK_MODEREADY             0x80
#define HRF_MASK_FIFONOTEMPTY          0x40
#define HRF_MASK_FIFOLEVEL             0x20
#define HRF_MASK_FIFOOVERRUN           0x10
#define HRF_MASK_PACKETSENT            0x08
#define HRF_MASK_TXREADY               0x20
#define HRF_MASK_PACKETMODE            0x60
#define HRF_MASK_MODULATION            0x18
#define HRF_MASK_PAYLOADRDY            0x04

// Radio modes
#define HRF_MODE_STANDBY               0x04     // Standby
#define HRF_MODE_TRANSMITTER           0x0C     // Transmiter
#define HRF_MODE_RECEIVER              0x10     // Receiver

// Values to store in registers
#define HRF_VAL_DATAMODUL_FSK          0x00     // Modulation scheme FSK
#define HRF_VAL_DATAMODUL_OOK          0x08     // Modulation scheme OOK
#define HRF_VAL_FDEVMSB30              0x01     // frequency deviation 5kHz 0x0052 -> 30kHz 0x01EC
#define HRF_VAL_FDEVLSB30              0xEC     // frequency deviation 5kHz 0x0052 -> 30kHz 0x01EC
#define HRF_VAL_FRMSB434               0x6C     // carrier freq -> 434.3MHz 0x6C9333
#define HRF_VAL_FRMID434               0x93     // carrier freq -> 434.3MHz 0x6C9333
#define HRF_VAL_FRLSB434               0x33     // carrier freq -> 434.3MHz 0x6C9333
#define HRF_VAL_FRMSB433               0x6C     // carrier freq -> 433.92MHz 0x6C7AE1
#define HRF_VAL_FRMID433               0x7A     // carrier freq -> 433.92MHz 0x6C7AE1
#define HRF_VAL_FRLSB433               0xE1     // carrier freq -> 433.92MHz 0x6C7AE1
#define HRF_VAL_AFCCTRLS               0x00     // standard AFC routine
#define HRF_VAL_AFCCTRLI               0x20     // improved AFC routine
#define HRF_VAL_LNA50                  0x08     // LNA input impedance 50 ohms
#define HRF_VAL_LNA50G                 0x0E     // LNA input impedance 50 ohms, LNA gain -> 48db
#define HRF_VAL_LNA200                 0x88     // LNA input impedance 200 ohms
#define HRF_VAL_RXBW60                 0x43     // channel filter bandwidth 10kHz -> 60kHz  page:26
#define HRF_VAL_RXBW120                0x41     // channel filter bandwidth 120kHz
#define HRF_VAL_AFCFEIRX               0x04     // AFC is performed each time RX mode is entered
#define HRF_VAL_RSSITHRESH220          0xDC     // RSSI threshold 0xE4 -> 0xDC (220)
#define HRF_VAL_PREAMBLELSB3           0x03     // preamble size LSB 3
#define HRF_VAL_PREAMBLELSB5           0x05     // preamble size LSB 5
#define HRF_VAL_SYNCCONFIG0            0x00     // sync word disabled
#define HRF_VAL_SYNCCONFIG1            0x80     // 1 byte  of tx sync
#define HRF_VAL_SYNCCONFIG2            0x88     // 2 bytes of tx sync
#define HRF_VAL_SYNCCONFIG3            0x90     // 3 bytes of tx sync
#define HRF_VAL_SYNCCONFIG4            0x98     // 4 bytes of tx sync
#define HRF_VAL_PAYLOADLEN255          0xFF     // max Length in RX, not used in Tx
#define HRF_VAL_PAYLOADLEN66           66       // max Length in RX, not used in Tx
#define HRF_VAL_FIFOTHRESH1            0x81     // Condition to start packet transmission: at least one byte in FIFO
#define HRF_VAL_FIFOTHRESH30           0x1E     // Condition to start packet transmission: wait for 30 bytes in FIFO


//===== RADIO CONFIGS ==========================================================


//----- CURRENT COST FSK -------------------------------------------------------
// The CC IAM sends 00 00 66 66 66 66 2D D4
// the 66's are really for receive bit synchroniser lock
// but we get some other random packets with just 2D D4,
// so including some of the 66's improves rejection of duff packets.

#define RADIO_VAL_CC_SYNCVALUE1             0x66
#define RADIO_VAL_CC_SYNCVALUE2             0x2D
#define RADIO_VAL_CC_SYNCVALUE3             0xD4

//#define RADIO_VAL_CC_PACKETCONFIG1          0x00            // Fixed length, no Manchester, no addr
#define RADIO_VAL_CC_PACKETCONFIG1          0x20            // 0010 0000 Fixed length, Manchester decoding, no addr
#define RADIO_VAL_CC_FRMSB                  0x6C            // 433.91mHz
#define RADIO_VAL_CC_FRMID                  0x7A
#define RADIO_VAL_CC_FRLSB                  0x3D

static const RFM69_CONFIG_REC _config_CC_FSK[] = {
    // RFM69HCW (high power)
    {HRF_ADDR_PALEVEL,        0x7F},                        // RFM69HCW high power PA_BOOST PA1+PA2
    {HRF_ADDR_OCP,            0x00},                        // RFM69HCW over current protect off
    // RFM69 (low power)
    //{HRF_ADDR_PALEVEL,        0x9F},                      // RMF69 (Energenie RT board) 13dBm, PA0=ON

    // FSK specific
    {HRF_ADDR_DATAMODUL,        HRF_VAL_DATAMODUL_FSK},     // FSK + packet mode
    //{HRF_ADDR_DATAMODUL,          0x40},                    // FSK + continuous, with bit sync (10)
    {HRF_ADDR_AFCCTRL,            HRF_VAL_AFCCTRLS},        // standard AFC routine
    {HRF_ADDR_LNA,                HRF_VAL_LNA50},           // 200ohms, gain by AGC loop -> 50ohms
    //{HRF_ADDR_RSSITHRESH,       0xF0},                    // 120*2
    {HRF_ADDR_FDEVMSB,            HRF_VAL_FDEVMSB30},       // frequency deviation 5kHz 0x0052 -> 30kHz 0x01EC
    {HRF_ADDR_FDEVLSB,            HRF_VAL_FDEVLSB30},       // frequency deviation 5kHz 0x0052 -> 30kHz 0x01EC
    {HRF_ADDR_FRMSB,              RADIO_VAL_CC_FRMSB},      // carrier freq -> 433.91MHz
    {HRF_ADDR_FRMID,              RADIO_VAL_CC_FRMID},      // carrier freq
    {HRF_ADDR_FRLSB,              RADIO_VAL_CC_FRLSB},      // carrier freq
    {HRF_ADDR_RXBW,               HRF_VAL_RXBW60},          // channel filter bandwidth 10kHz -> 60kHz  page:26
    {HRF_ADDR_BITRATEMSB,         0x1E},                    // 32MHz/4096 = 7813 = 1E85
    {HRF_ADDR_BITRATELSB,         0x85},                    //
    {HRF_ADDR_SYNCCONFIG,         HRF_VAL_SYNCCONFIG3},     // 3 bytes of sync word
    {HRF_ADDR_SYNCVALUE1,         RADIO_VAL_CC_SYNCVALUE1}, // 1st byte of Sync word
    {HRF_ADDR_SYNCVALUE2,         RADIO_VAL_CC_SYNCVALUE2}, // 2nd byte of Sync word
    {HRF_ADDR_SYNCVALUE3,         RADIO_VAL_CC_SYNCVALUE3}, // 3rd byte of Sync word
    {HRF_ADDR_PACKETCONFIG1,      RADIO_VAL_CC_PACKETCONFIG1},

    {HRF_ADDR_PAYLOADLEN,         8}                         // Fixed number of receive bytes (manchester encoded doubles it)
//    {HRF_ADDR_NODEADRS,           0x06},                    // Node address used in address filtering (not used)
};
#define CONFIG_CC_FSK_COUNT (sizeof(_config_CC_FSK)/sizeof(RFM69_CONFIG_REC))

static const struct
{
    RFM69_CONFIG_REC const * config;
    uint8_t                count;
} _configs[] =
{
    /* RFM69_CONFIG_CC_FSK  */ {_config_CC_FSK,  CONFIG_CC_FSK_COUNT}
};
#define NUM_CONFIGS (sizeof(_configs)/sizeof(RFM69_CONFIG_REC *))


//------------------------------------------------------------------------------
// Read a single register from the radio chip.
//
// 'reg': The register to read.
// returns: An 8 bit value from the register.
//
// note: SPI does not provide any level of error checking, so if the
// chip is not responding, the bus will probably return 0x00 or 0xFF.

static uint8_t _readreg(uint8_t addr)
{
    uint8_t result;

    spi_select();
    spi_byte(addr);
    result = spi_byte(0x00);
    spi_deselect();
    return result;
}

//------------------------------------------------------------------------------
static bool _checkreg(uint8_t addr, uint8_t mask, uint8_t value)
{
    uint8_t regval = _readreg(addr);
    return ((regval & mask) == value);
}


//------------------------------------------------------------------------------
static void _waitreg(uint8_t addr, uint8_t mask, uint8_t value)
{
    while (! _checkreg(addr, mask, value))
    {
      // busy wait
      //TODO: No timeout or error recovery? Can cause permanent lockup
    }
}


//------------------------------------------------------------------------------
static void _writereg(uint8_t addr, uint8_t data)
{
    spi_select();
    spi_byte(addr | HRF_MASK_WRITE_DATA);
    spi_byte(data);
    spi_deselect();
}


//------------------------------------------------------------------------------
void _clear_fifo(void)
{
    //TODO: max fifolen is 66, should bail after that to prevent lockup
    //especially if radio crashed and SPI always returns stuck flag bit
    while ((_readreg(HRF_ADDR_IRQFLAGS2) & HRF_MASK_FIFONOTEMPTY) == HRF_MASK_FIFONOTEMPTY)
    {
        _readreg(HRF_ADDR_FIFO);
    }
}


//------------------------------------------------------------------------------
static void _writefifo_burst(uint8_t* buf, uint8_t len)
{
    spi_select();
    spi_byte(HRF_ADDR_FIFO | HRF_MASK_WRITE_DATA);
    spi_bytes(buf, NULL, len);
    spi_deselect();
}


//------------------------------------------------------------------------------
static void _wait_ready(void)
{
    _waitreg(HRF_ADDR_IRQFLAGS1, HRF_MASK_MODEREADY, HRF_MASK_MODEREADY);
}


//------------------------------------------------------------------------------
static void _wait_tx_ready(void)
{
    _waitreg(HRF_ADDR_IRQFLAGS1, HRF_MASK_MODEREADY|HRF_MASK_TXREADY, HRF_MASK_MODEREADY|HRF_MASK_TXREADY);
}


//------------------------------------------------------------------------------
static void _config(RFM69_CONFIG_REC const * p_config, uint8_t len)
{
    while (len-- != 0)
    {
        _writereg(p_config->addr, p_config->value);
        p_config++;
    }
}


//===== PUBLIC =================================================================

//------------------------------------------------------------------------------
RFM69_RESULT rfm69_init(void)
{
    _rfm69_data.radiover = _readreg(HRF_ADDR_VERSION);
    _rfm69_data.config   = RFM69_CONFIG_NONE;

    if ((0x00 == _rfm69_data.radiover) || (0xFF == _rfm69_data.radiover))
    {
        return RFM69_RESULT_E_NORESPONSE;
    }
    return RFM69_RESULT_OK;
}

//------------------------------------------------------------------------------
uint8_t rfm69_radiover(void)
{
    return _rfm69_data.radiover;
}

//------------------------------------------------------------------------------
RFM69_RESULT rfm69_setmode(RFM69_MODE mode)
{
    // Prevent HRF_MODE leaking via API
    static uint8_t _hrf_modes[] =
    {
        /* RFM69_MODE_STBY */ HRF_MODE_STANDBY,
        /* RFM69_MODE_RX   */ HRF_MODE_RECEIVER,
        /* RFM69_MODE_TX   */ HRF_MODE_TRANSMITTER
    };
    if (mode >= sizeof(_hrf_modes)) {return RFM69_RESULT_E_INVALID_PARAMETER;}

    uint8_t hrf_mode = _hrf_modes[mode];
    _writereg(HRF_ADDR_OPMODE, hrf_mode);
    _wait_ready();

    if (hrf_mode == HRF_MODE_TRANSMITTER)
    {
        _wait_tx_ready();
    }
    //else if (mode == HRF_MODE_RECEIVER)
    //{
    //}

    _rfm69_data.current_mode = mode;
    return RFM69_RESULT_OK;
}


//------------------------------------------------------------------------------
RFM69_RESULT rfm69_setconfig(RFM69_CONFIG config)
{
    if ( config >= NUM_CONFIGS)
    {
        return RFM69_RESULT_E_INVALID_PARAMETER;
    }

    // only reconfigure if it actually changes
    if (config != _rfm69_data.config)
    {
        //ser_txstr("new config\r\n");
        _config(_configs[config].config, _configs[config].count);
        _rfm69_data.config = config;
    }
    return RFM69_RESULT_OK;
}

//------------------------------------------------------------------------------
RFM69_CONFIG rfm69_getconfig(void)
{
    return _rfm69_data.config;
}

//------------------------------------------------------------------------------
RFM69_RESULT rfm69_tx(uint8_t * ppayload, uint8_t len, uint8_t times)
{
    /* VALIDATE: Check input parameters are in range */
    if (0 == len) //TODO: make this an ASSERT()
    {
        return RFM69_RESULT_E_INVALID_PARAMETER;
    }
    if (0 == times)
    {
        times = DEFAULT_TIMES;
    }
    if (len > MAX_FIFO_BUFFER)
    {
        return RFM69_RESULT_E_BUFFER_TOO_SMALL;
    }

    uint8_t prevmode = _rfm69_data.current_mode;
    if (prevmode != RFM69_MODE_TX)
    {
        rfm69_setmode(RFM69_MODE_TX);
    }

    // Note, when PA starts up, radio inserts a 01 at start before any user data
    // we might need to pad away from this by sending a sync of many zero bits
    // to prevent it being misinterpreted as a preamble, and prevent it causing
    // the first bit of the preamble being twice the length it should be in the
    // first packet.
    // Also need to confirm this bit only occurs when transmit actually starts,
    // and not on every FIFO load.

    /* CONFIGURE: Setup the radio for transmit of the correct payload length */
    // Start transmitting when a full payload is loaded. So for '15':
    // level triggers when it 'strictly exceeds' level (i.e. 16 bytes starts tx,
    // and <=15 bytes triggers fifolevel irqflag to be cleared)
    // We already know from earlier that payloadlen<=32 (which fits into half a FIFO)
    _writereg(HRF_ADDR_FIFOTHRESH, len-1);


    /* TRANSMIT: Transmit a number of payloads back to back */

    // send a number of payload repeats for the whole packet burst
    for (uint8_t i=0; i<times; i++)
    {
        _writefifo_burst(ppayload, len);
        // Tx will auto start when fifolevel is exceeded by loading the payload
        // so the level register must be correct for the size of the payload
        // otherwise transmit will never start.
        /* wait for FIFO to not exceed threshold level */
        _waitreg(HRF_ADDR_IRQFLAGS2, HRF_MASK_FIFOLEVEL, 0);
    }

    // wait for FIFO empty, to indicate transmission completed
    _waitreg(HRF_ADDR_IRQFLAGS2, HRF_MASK_FIFONOTEMPTY, 0);

    // Check final flags in case of overruns etc
    //uint8_t irqflags1 = _readreg(HRF_ADDR_IRQFLAGS1);
    uint8_t irqflags2 = _readreg(HRF_ADDR_IRQFLAGS2);

    if (_rfm69_data.current_mode != prevmode)
    {
       rfm69_setmode(prevmode);
    }

    /* CONFIRM: Was the transmit ok? */
    if (((irqflags2 & HRF_MASK_FIFONOTEMPTY) != 0) || ((irqflags2 & HRF_MASK_FIFOOVERRUN) != 0))
    {
        return RFM69_RESULT_E_OVERRUN;
    }
    return RFM69_RESULT_OK;
}


//------------------------------------------------------------------------------
RFM69_RESULT rfm69_receive_waiting(void)
{
    uint8_t irqflags2 = _readreg(HRF_ADDR_IRQFLAGS2);

    if ((irqflags2 & HRF_MASK_PAYLOADRDY) == HRF_MASK_PAYLOADRDY)
    {
        return RFM69_RESULT_I_READY;
    }
    else
    {
        return RFM69_RESULT_I_NOTREADY;
    }
}


//------------------------------------------------------------------------------
// read a single payload from the payload buffer
// this reads count byte preceeded payloads.
// The CBP payload always has the count byte in the first byte
// and this is returned in the user buffer too.
// Read bytes from FIFO in burst mode.
// Never reads more than buflen bytes
// First received byte is the count of remaining bytes
// That byte is also returned in the user buffer.
// Note the user buffer can be > FIFO_MAX, but there is no flow control
// in the HRF driver yet, so you might get an underflow error if data is read
// quicker than it comes in on-air. You might get an overflow error if
// data comes in quicker than it is read.

RFM69_RESULT rfm69_rxcbp(uint8_t * ppayload, uint8_t maxlen)
{
    uint8_t data;

    spi_select();
    spi_byte(HRF_ADDR_FIFO); /* prime the fifo burst reader */

    /* Read the first byte, and then decide how many remaining bytes to receive */
    data = spi_byte(HRF_ADDR_FIFO);
    *(ppayload++) = data; /* the count byte is always returned as first byte of user buffer */

    /* Validate the payload len against the supplied user buffer */
    if (data > maxlen)
    {
        spi_deselect();
        return RFM69_RESULT_E_BUFFER_TOO_SMALL;
    }

    maxlen = data; /* now the expected payload length */

    while (maxlen != 0)
    {
        data = spi_byte(HRF_ADDR_FIFO);
        *(ppayload++) = data;
        maxlen--;
    }
    spi_deselect();

    //TODO: Read irqflags
    //if underflow, this is an error (reading out too quick)
    //if overflow, this is an error (not reading out quick enough)
    //if not empty at end, this is a warning (might be ok, but user might want to clear_fifo after)
    return RFM69_RESULT_OK;
}


//------------------------------------------------------------------------------
RFM69_RESULT rfm69_rx(uint8_t * ppayload, uint8_t maxlen)
{
    if (maxlen > MAX_FIFO_BUFFER)
    {  /* At the moment, the receiver cannot reliably cope with payloads > 1 FIFO buffer.
        * It *might* be able to in the future.
        */
        return RFM69_RESULT_E_LONG_PAYLOAD;
    }

    uint8_t data;

    spi_select();
    spi_byte(HRF_ADDR_FIFO); /* prime the fifo burst reader */

    while (maxlen != 0)
    {
        data = spi_byte(HRF_ADDR_FIFO);
        *(ppayload++) = data;
        maxlen--;
    }
    spi_deselect();

    //TODO: Read irqflags
    //if underflow, this is an error (reading out too quick)
    //if overflow, this is an error (not reading out quick enough)
    //if not empty at end, this is a warning (might be ok, but user might want to clear_fifo after)
    return RFM69_RESULT_OK;
}

// END
