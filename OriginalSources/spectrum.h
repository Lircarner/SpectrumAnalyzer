#include "types.h"

// commands
#define RESP_ACK		1
#define RESP_NAK		0
#define CMD_POLL		0x10
#define CMD_PIR			0x11

#define CMD_STATUS		0x14
#define CMD_SHDN		0x15
#define CMD_CONTROL		0x16
#define CMD_AUTO_SHDN	0x17

// port bits
#define RF_PORT			PORTB
#define RF_CS_BIT		2
#define RF_MOSI_BIT		3
#define RF_MISO_BIT		4
#define RF_SCK_BIT 		5

#define DEBUG_PORT		PORTD
#define DEBUG1_BIT		2
#define DEBUG2_BIT		3

#define PORTB_DDR		0x2f
#define PORTB_INIT		0x04
#define PORTC_DDR		0xff
#define PORTC_INIT		0x00
#define PORTD_DDR		0xfe
#define PORTD_INIT		0x03

void cc2500Read(uint8 reg, uint8* data);
void cc2500Write(uint8 reg, uint8 data);
void waitUS(unsigned short us);
void waitMS(unsigned short ms);
void hwInit(void);
void sendString(char* t);
unsigned char eepromRead( int location);
void eepromWrite( int location, unsigned char byte);
