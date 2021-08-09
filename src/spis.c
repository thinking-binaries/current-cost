/* spis.c  D.J.Whale  19/07/2014
 *
 * Software SPI driver
 */

#include <stdint.h>
#include <stdlib.h>
#include "spi.h"
#include "port.h"
#include "timer.h"

// CS
// CSPOL0 = low when idle, high when active
// CSPOL1 = high when idle, low when active
#define DESELECT() do { if (_mode & SPI_CSPOL1) SPI_CS_HIGH(); else SPI_CS_LOW();} while (0)
#define SELECT()   do { if (_mode & SPI_CSPOL1) SPI_CS_LOW();  else SPI_CS_HIGH();} while (0)

// CLOCK
// CPOL0 = low when idle, high when active
// CPOL1 = high when idle, low when active
#define CLOCK_IDLE()   do { if (_mode & SPI_CPOL1) SPI_SCLK_HIGH(); else SPI_SCLK_LOW();} while (0)
#define CLOCK_ACTIVE() do { if (_mode & SPI_CPOL1) SPI_SCLK_LOW();  else SPI_SCLK_HIGH();} while (0)

// Timing
#define DELAY_FREQ_US      1
#define DELAY_CS_SETTLE_US 1
#define DELAY_TX_SETTLE_US 1

static uint8_t _mode = 0;


//------------------------------------------------------------------------------
void spi_init(uint8_t mode)
{
    _mode = mode;
    SPI_SCLK_AS_OUT();
    CLOCK_IDLE();

    SPI_MOSI_AS_OUT();
    SPI_MOSI_LOW();
    SPI_MISO_AS_IN();

    DESELECT();
    SPI_CS_AS_OUT();
}


//------------------------------------------------------------------------------
void spi_finished(void)
{
    SPI_MOSI_AS_IN();
    SPI_CS_AS_IN();
}


//------------------------------------------------------------------------------
void spi_select(void)
{
    SELECT();
    timer_delay_us(DELAY_CS_SETTLE_US);
}


//------------------------------------------------------------------------------
void spi_deselect(void)
{
    DESELECT();
}


//------------------------------------------------------------------------------
uint8_t spi_byte(uint8_t txbyte)
{
    uint8_t rxbyte = 0;
    uint8_t bitno;
    uint8_t bit ;

    //NOTE: This is for CPHA0 only
    //TODO: Implement CPHA1

    for (bitno=0; bitno<8; bitno++)
    {
        /* Transmit MSB first */
        bit = ((txbyte & 0x80) != 0x00);
        txbyte <<= 1;
        SPI_MOSI_WRITE(bit);
        timer_delay_us(DELAY_TX_SETTLE_US);
        CLOCK_ACTIVE();
        timer_delay_us(DELAY_FREQ_US);

        /* Read MSB first */
        bit = SPI_MISO_READ();
        rxbyte = (rxbyte<<1) | bit;

        CLOCK_IDLE();
        timer_delay_us(DELAY_FREQ_US);
    }
    return rxbyte;
}


//------------------------------------------------------------------------------
void spi_bytes(uint8_t* pTx, uint8_t* pRx, uint8_t count)
{
    uint8_t tx = 0;
    uint8_t rx;

    while (count > 0)
    {
        if (pTx != NULL)
        {
            tx = *(pTx++);
        }
        rx = spi_byte(tx);
        if (pRx != NULL)
        {
            *(pRx++) = rx;
        }
        count--;
    }
}


//------------------------------------------------------------------------------
void spi_frame(uint8_t* pTx, uint8_t* pRx, uint8_t count)
{
    SELECT();
    timer_delay_us(DELAY_CS_SETTLE_US);
    spi_bytes(pTx, pRx, count);
    DESELECT();
}


/***** END OF FILE *****/
