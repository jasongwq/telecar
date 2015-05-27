#ifndef _SPI_H
#define _SPI_H

#include "STC15W401.h"

#define WRITE		0x7F
#define READ		0x80

sbit	SS 		= P4^3;       		//output
sbit	SCLK	= P2^1;     		//output
sbit	MOSI 	= P2^3;     		//output
sbit	MISO 	= P2^2;       		//input
sbit	RESET_N	= P0^7;  			//output
sbit	PKT 	= P0^6;  			//input

sbit	SS2 		= P0^0;       		//output
sbit	SCLK2	  = P0^1;     		//output
sbit	MOSI2 	= P0^2;     		//output
sbit	MISO2 	= P0^3;       		//input
sbit	RESET2_N= P0^4;  			//output
sbit	PKT2 	  = P0^5;  			//input


void InitLT8900(void);
void spiWriteReg(unsigned char reg, unsigned char byteH, unsigned char byteL);
void spiReadreg(unsigned char reg);
unsigned char spiReadWrite(unsigned char Byte);

#endif
