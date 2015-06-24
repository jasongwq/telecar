#ifndef _SPI51_H
#define _SPI51_H

#ifdef __cplusplus
       extern "C" {
#endif


#define WRITE		0x7F
#define READ		0x80




void InitLT8900(void);
void spiWriteReg(unsigned char reg, unsigned char byteH, unsigned char byteL);
void spiReadreg(unsigned char reg);
unsigned char spiReadWrite(unsigned char Byte);
				 #ifdef __cplusplus
        }
#endif
#endif
