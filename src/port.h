// port.h  D.J.Whale
//
// Mapping of port pins, and macros to access them.

#ifndef _PORT_H
#define _PORT_H

#include <avr/io.h>


//----- PIN 1 -----------------------------------------------------------------
//PB5 pin 1 RESET/dW            85: (PB5/PCINT5/nRESET/ADC0/dW)

// Not used, reserved for RESET and debug wire.
// Note, RESET fuse is required in reset mode for low voltage programmer.
// This also reserves the pin for later debug wire use if needed.


//----- PIN 2 -----------------------------------------------------------------
//PB3 pin 2  CS (active low)    85: (PB3/PCINT3/XTAL1/CLKI/nOC1B/ADC3)
//
// This is a software SPI port

#define SPI_CS           3
#define SPI_CS_MASK      (1<<SPI_CS)
#define SPI_CS_AS_OUT()  DDRB  |=   SPI_CS_MASK
#define SPI_CS_AS_IN()   DDRB  &= ~ SPI_CS_MASK
#define SPI_CS_HIGH()    PORTB |=   SPI_CS_MASK
#define SPI_CS_LOW()     PORTB &= ~ SPI_CS_MASK
#define SPI_CS_WRITE(B)  do{ if (B) SPI_CS_HIGH(); else SPI_CS_LOW();} while (0)



//----- PIN 3 -----------------------------------------------------------------
// PB4 pin 3 MISO               85: (PB4/PCINT4/XTAL2/CLKO/OC1B/ADC2)
//
// This is a software SPI port

#define SPI_MISO           4
#define SPI_MISO_MASK      (1<<SPI_MISO)
#define SPI_MISO_AS_IN()   DDRB &= ~ SPI_MISO_MASK
#define SPI_MISO_IS_LOW()  ((PINB & SPI_MISO_MASK) == 0x00)
#define SPI_MISO_IS_HIGH() ((PINB & SPI_MISO_MASK) != 0x00)
#define SPI_MISO_READ()    SPI_MISO_IS_HIGH()
#define SPI_MISO_AS_OUT()  DDRB |= SPI_MISO_MASK
#define SPI_MISO_LOW()     PORTB &= ~ SPI_MISO_MASK
#define SPI_MISO_HIGH()    PORTB |= SPI_MISO_MASK


//----- PIN 4 -----------------------------------------------------------------
//GND VSS 0V


//----- PIN 5 -----------------------------------------------------------------
//PB0 pin 5 MOSI                85: (PB0/MOSI/DI/SDA/AIN0/OC0A/nOC1A/AREF/PCINT0)
//
// This is a software SPI port

#define SPI_MOSI           0
#define SPI_MOSI_MASK      (1<<SPI_MOSI)
#define SPI_MOSI_AS_IN()   DDRB &= ~ SPI_MOSI_MASK
#define SPI_MOSI_IS_LOW()  ((PINB & SPI_MOSI_MASK) == 0x00)
#define SPI_MOSI_IS_HIGH() ((PINB & SPI_MOSI_MASK) != 0x00)
#define SPI_MOSI_READ()    SPI_MOSI_IS_HIGH()
#define SPI_MOSI_AS_OUT()  DDRB |= SPI_MOSI_MASK
#define SPI_MOSI_LOW()     PORTB &= ~ SPI_MOSI_MASK
#define SPI_MOSI_HIGH()    PORTB |= SPI_MOSI_MASK
#define SPI_MOSI_WRITE(B)  do{ if (B) SPI_MOSI_HIGH(); else SPI_MOSI_LOW();} while (0)


//----- PIN 6 -----------------------------------------------------------------
//PB1 pin 6  APP PIN            85:(PB1/MISO/D0/AIN1/OC0B/OC1A/PCINT1)

#define APP            1
#define APP_MASK       (1<<APP)
#define APP_AS_IN()    DDRB &= ~ APP_MASK
#define APP_IS_LOW()   ((PINB & APP_MASK) == 0)
#define APP_IS_HIGH()  ((PINB & APP_MASK) != 0)
#define APP_READ()     APP_IS_HIGH()
#define APP_AS_OUT()   DDRB |= APP_MASK
#define APP_LOW()      PORTB &= ~ APP_MASK
#define APP_HIGH()     PORTB |=  APP_MASK
#define APP_TOGGLE()   PORTB ^= APP_MASK
#define APP_WRITE(B)   do{ if (B) APP_HIGH(); else APP_LOW();} while (0)

// This is a half duplex software serial port
//SERIAL TX
#define SER_AS_OUT()   APP_AS_OUT()
#define SER_HIGH()     APP_HIGH()
#define SER_LOW()      APP_LOW()
#define SER_TOGGLE()   APP_TOGGLE()
#define SER_WRITE(B)   APP_WRITE(B)

//SERIAL RX
#define SER_AS_IN()    APP_AS_IN()
#define SER_IS_LOW()   APP_IS_LOW()
#define SER_IS_HIGH()  APP_IS_HIGH()
#define SER_READ()     APP_READ()

// If the serial is disabled, the OC0B is routed here for PWM
//PWM
#define PWM            APP
#define PWM_AS_OUT()   APP_AS_OUT()
#define PWM_AS_IN()    APP_AS_IN()
#define PWM_LOW()      APP_LOW()
#define PWM_HIGH()     APP_HIGH()


//----- PIN 7 -----------------------------------------------------------------
//PB2 pin 7 SCLK                85: (PB2/SCK/USCK/SCL/ADC1/T0/INT0/PCINT2)

// This is a software SPI port

#define SPI_SCLK           2
#define SPI_SCLK_MASK      (1<<SPI_SCLK)
#define SPI_SCLK_AS_IN()   DDRB &= ~ SPI_SCLK_MASK
#define SPI_SCLK_IS_LOW()  ((PINB & SPI_SCLK_MASK) == 0x00)
#define SPI_SCLK_IS_HIGH() ((PINB & SPI_SCLK_MASK) != 0x00)
#define SPI_SCLK_READ()    SPI_SCLK_IS_HIGH()
#define SPI_SCLK_AS_OUT()  DDRB |= SPI_SCLK_MASK
#define SPI_SCLK_LOW()     PORTB &= ~ SPI_SCLK_MASK
#define SPI_SCLK_HIGH()    PORTB |= SPI_SCLK_MASK
#define SPI_SCLK_WRITE(B)  do{ if (B) SPI_SCLK_HIGH(); else SPI_SCLK_LOW();} while (0)


//----- PIN 8 -----------------------------------------------------------------
// VCC 3V

#endif

//end: port.h
