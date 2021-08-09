/* spi.h  D.J.Whale  19/07/2014 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>

// clock polarity
#define SPI_CPOL0 0x00
#define SPI_CPOL1 0x01

// clock phase
#define SPI_CPHA0 0x00
#define SPI_CPHA1 0x02

// select polarity
#define SPI_CSPOL0 0x00
#define SPI_CSPOL1 0x04


/***** FUNCTION PROTOTYPES *****/

void spi_init(uint8_t mode);

void spi_select(void);

void spi_deselect(void);

uint8_t spi_byte(uint8_t txbyte);

void spi_bytes(uint8_t* pTx, uint8_t* pRx, uint8_t count);

void spi_frame(uint8_t* pTx, uint8_t* pRx, uint8_t count);

void spi_finished(void);

#endif

/***** END OF FILE *****/
