/*================================================================================
	Low Cost Spectrum Analyzer
	Embedded Software
	spectrum.c

	Copyright ©2005 Scott Armitage.

THIS SOFTWARE AND DOCUMENTATION IS PROVIDED "AS IS," AND COPYRIGHT HOLDER MAKES NO
REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO,
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE
USE OF THE SOFTWARE OR DOCUMENTATION WILL NOT INFRINGE ANY THIRD PARTY PATENTS,
COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS.

COPYRIGHT HOLDER WILL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THE SOFTWARE OR DOCUMENTATION.

You may freely copy and redistribute this software if no fee is charged for use,
copying or distribution.  Such redistributions must retain the above copyright
notice and disclaimer.
================================================================================*/

#include <iom88v.h>
#include <macros.h>
#include <stdio.h>
#include "types.h"
#include "spectrum.h"
#include "comms.h"

#define TRUE 1

uint8 cal[256];

// EEPROM addresses
#define EEPROM_RSSI_OFS		10
#define EEPROM_XTAL_FREQ	11

// EEPROM defaults
#define EEPROM_RSSI_DEF		190
#define EPROM_XTAL_F_DEF	27

/*****************************************************************************************

	main

*****************************************************************************************/
main(void)
{
	uint8 data, cmd, reg, n1, n2, max, rssiOfs;
	uint16 ms, i, j;

	hwInit();											// setup hardware, regs

	for (i=0; i<256; i++)
		cal[i] = 20;									// init cal values

	rssiOfs = eepromRead(EEPROM_RSSI_OFS);				// load cal from eeprom
	if (rssiOfs == 0xff) {
		rssiOfs = EEPROM_RSSI_DEF;
		eepromWrite(EEPROM_RSSI_OFS, rssiOfs);			// init eeprom
		eepromWrite(EEPROM_XTAL_FREQ, EPROM_XTAL_F_DEF);
	}

	DEBUG_PORT |= BIT(DEBUG2_BIT);						// led - power indicator

	while (TRUE) {
		if (dataAvail()) {
			rcvByte(&data);
			if (data == SOH) {
				DEBUG_PORT &= ~BIT(DEBUG2_BIT);			// blink led - activity

				while (!dataAvail())
					;
				rcvByte(&cmd);

				if (cmd == 'S') {						// get spectrum
					while(!dataAvail())
						;
					rcvByte(&n1);						// first channel

					while(!dataAvail())
						;
					rcvByte(&n2);						// last channel

					while(!dataAvail())
						;
					rcvByte(&data);						// delay between samples in 250 us increments (0=no delay)
					ms = data;

					cc2500Write(0x0a, n1);				// set channel
					cc2500Write(0x31, 0x3d);			// calibrate and wait
					waitUS(800);
					cc2500Write(0x34, 0x3d);			// enable rx

					for (i=n1; i<=n2; i++) {
						cc2500Write(0x0a, i);			// set channel
						cc2500Write(0x25, cal[i]);		// calibration value
						waitUS(300);					// settling time

						max = 0;
						for (j=0; j<=5*ms; j++) {		// oversample - save maximum
							waitUS(50);
							cc2500Read(0xf4, &data);	// read RSSI
							data -= rssiOfs;			// apply offset
							if (data > max)				// oversample and keep maximum
								max = data;
						}
						sendByte(max);					// send to host as we acquire
					}
				}

				if (cmd == 'R') {						// register read
					while(!dataAvail())
						;
					rcvByte(&reg);						// register to read

					cc2500Read(reg, &data);				// get data

					sendByte(data);
				}

				if (cmd == 'W') {						// register write
					while(!dataAvail())
						;
					rcvByte(&reg);						// register to read

					while(!dataAvail())
						;
					rcvByte(&data);						// data to write

					cc2500Write(reg, data);				// write it

					sendByte('A');						// ack
				}

				if (cmd == 'C') {						// calibrate
					for (i=0; i<256; i++) {
						cc2500Write(0x0a, i);			// set channel
						cc2500Write(0x36, 0x3d);		// idle mode
						cc2500Write(0x33, 0x3d);		// manual calibration
						waitUS(800);					// wait for cal
						cc2500Read(0x25, &data);
						cal[i] = data;					// save cal value
					}

					sendByte('A');						// ack
				}

				if (cmd == 'r') {						// eeprom read
					while(!dataAvail())
						;
					rcvByte(&reg);						// eeprom address

					data = eepromRead(reg);				// do read

					sendByte(data);
				}

				if (cmd == 'w') {						// eeprom write
					while(!dataAvail())
						;
					rcvByte(&reg);						// eeprom address

					while(!dataAvail())
						;
					rcvByte(&data);						// eeprom data

					eepromWrite(reg, data);				// do write

					sendByte('A');						// ack
				}
			}
		}
		DEBUG_PORT |= BIT(DEBUG2_BIT);					// turn on led
	}

	return 0;
}


/*****************************************************************************************

	cc2500Read

*****************************************************************************************/
void cc2500Read(uint8 reg, uint8* data)
{
	RF_PORT &= ~BIT(RF_CS_BIT);				// CS = 0

	SPDR = reg | 0x80;
	while(!(SPSR & BIT(SPIF)))
		;
	SPDR = 0;
	while(!(SPSR & BIT(SPIF)))
		;
	*data = SPDR;

	RF_PORT |= BIT(RF_CS_BIT);				// CS = 1
}


/*****************************************************************************************

	cc2500Write

*****************************************************************************************/
void cc2500Write(uint8 reg, uint8 data)
{
	RF_PORT &= ~BIT(RF_CS_BIT);				// CS = 0

	SPDR = reg & 0x7f;
	while(!(SPSR & BIT(SPIF)))
		;
	SPDR = data;
	while(!(SPSR & BIT(SPIF)))
		;

	RF_PORT |= BIT(RF_CS_BIT);				// CS = 1
}


/*****************************************************************************************

	hwInit

*****************************************************************************************/
void hwInit()
{
	uint8 i;

	// ports
	PORTB = PORTB_INIT;
	DDRB = PORTB_DDR;
	PORTC = PORTC_INIT;
	DDRC = PORTC_DDR;
	PORTD = PORTD_INIT;
	DDRD = PORTD_DDR;

	// uart
	UBRR0 = 12;						// 38400 baud
	UCSR0B = BIT(RXEN0) | BIT(TXEN0);

	// spi
	SPCR = 0x00;					// disable SPI

	// int cc2500
	RF_PORT |= BIT(RF_SCK_BIT);		// sclk = 1
	RF_PORT &= ~BIT(RF_MOSI_BIT);	// mosi = 0
	RF_PORT &= ~BIT(RF_CS_BIT);		// cs = 0
	RF_PORT |= BIT(RF_CS_BIT);		// cs = 1
	waitUS(40);
	RF_PORT &= ~BIT(RF_CS_BIT);		// cs = 0

	// spi
	SPSR = 0x01;					// 2X
	SPCR = 0x51;					// enable SPI, 1 Mhz clock

	waitUS(10000);

	SPDR = 0x30;					// SRES command
	while(!(SPSR & BIT(SPIF)))
		;
	RF_PORT |= BIT(RF_CS_BIT);		// cs = 1
}


/*****************************************************************************************

	waitMS

	waits for desired delay
	uses timer 1 with 1 us clock

	input:	ms:	delay in ms  [range: 0 to 65535]
*****************************************************************************************/
void waitMS(uint16 ms)
{
	uint16 i;

	for (i=0; i<ms; i++)
		waitUS(995);					// 1 ms
}


/*****************************************************************************************

	waitUS

	waits for desired delay
	uses timer 1 with 1 us clock

	input:	us:	delay in us  [range: 0 to 65535]
*****************************************************************************************/
void waitUS(uint16 us)
{
	TCCR1B = 2;							// 1 us clock
	TCNT1 = 0;

	if (us > 10) {
		TCNT1 = 0;
		while (TCNT1 < us)
			;
	}
}


/*****************************************************************************************

	sendString

	sends null-terminated string to UART
*****************************************************************************************/
void sendString(char* t)
{
	while (*t) {
		while ( !( UCSR0A & (1<<UDRE0)) )
			;
		UDR0 = *t;
		t++;
	}
}


/*****************************************************************************************

	eepromRead

	reads from eeprom
*****************************************************************************************/
unsigned char eepromRead( int location)
{
	EEAR = location;

    EECR |= 0x01;                       // Set READ strobe

    return (EEDR);                      // Return byte
}


/*****************************************************************************************

	eepromWrite

	writes to eeprom
*****************************************************************************************/
void eepromWrite( int location, unsigned char byte)
{
	unsigned char oldSREG;

	EEAR = location;

    EEDR = byte;

	oldSREG = SREG;
	SREG &= ~0x80;						// disable interrupt

    EECR |= 0x04;                       // Set MASTER WRITE enable
    EECR |= 0x02;                       // Set WRITE strobe
    while (EECR & 0x02);                // Wait until write is done

	SREG = oldSREG;
}
