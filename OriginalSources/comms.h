#define SOH 				0xA6
#define EOH					0xA7
#define EOD					0xA8

#define ACK					0x00
#define NAK_CMD				0xA0
#define NAK_CMD_EOH			0xA1
#define NAK_DATA_EOD		0xA2
#define NAK_DATA_TIMEOUT	0xA3

#define COMM_TIMEOUT	150		// number of ms before giving up on UART data


void sendByte(unsigned char data);
unsigned char dataAvail(void);
unsigned char rcvByte(unsigned char* data);
