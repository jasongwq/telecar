#ifndef _SPI_H
#define _SPI_H

#include "STC15W401.h"


#define WRITE		0x7F
#define READ		0x80

sbit	SS 		= P1^2;       		//output
sbit	SCLK	= P1^5;     		//output
sbit	MOSI 	= P1^3;     		//output
sbit	MISO 	= P1^4;       		//input
sbit	RESET_N	= P1^6;  			//output
sbit	PKT 	= P3^2;  			//input

void InitLT8900(void);
void spiWriteReg(unsigned char reg, unsigned char byteH, unsigned char byteL);
void spiReadreg(unsigned char reg);
unsigned char spiReadWrite(unsigned char Byte);
#endif
