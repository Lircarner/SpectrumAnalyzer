/*================================================================================
	Low Cost Spectrum Analyzer
	Embedded Software
	comms.c

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
#include "spectrum.h"
#include "comms.h"


/*******************************************************************************
	dataAvail

	Checks whether there is any received data to be read from the UART

	Returns:
		0=no data available, else data is available
*******************************************************************************/
unsigned char dataAvail(void)
{
	return (UCSR0A & BIT(RXC0));
}


/*******************************************************************************
	rcvByte

	Receives 1 byte from UART

	Input:
		data:	pointer to received data
	Returns:
		0=no error, 1=timeout error
*******************************************************************************/
unsigned char rcvByte(unsigned char* data)
{
	if (dataAvail()) {
		*data = UDR0;				// get the data
		return 0;
	}
	else
		return 1;					// timeout
}


/*******************************************************************************
	sendByte

	Transmits 1 byte via UART

	Input:
		data:	byte to send
*******************************************************************************/
void sendByte(unsigned char data)
{
	while (!(UCSR0A & BIT(UDRE0)))
		;									// wait if UART busy

	UDR0 = data;							// send data
}
