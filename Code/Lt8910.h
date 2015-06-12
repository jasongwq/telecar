#ifndef _LT8910_H
#define _LT8910_H


volatile unsigned char RegH, RegL;
volatile unsigned char Reg2H, Reg2L;

unsigned char spiReadWrite(unsigned char Byte)
{
    unsigned char i;

    EA = 0;
    for (i = 0; i < 8; i++)
    {
        MOSI = (Byte & 0x80);
        SCLK = 1;
        Delay10us();
        Byte <<= 1;
        Byte |= MISO;
        SCLK = 0;
        Delay10us();
    }
    EA = 1;

    return (Byte);
}

void spiWriteReg(unsigned char reg, unsigned char byteH, unsigned char byteL)
{
    SS = 0;
    spiReadWrite(WRITE & reg);
    spiReadWrite(byteH);
    spiReadWrite(byteL);
    SS = 1;
}
void spiReadreg(unsigned char reg)
{
    SS = 0;
    spiReadWrite(READ | reg);
    RegH = spiReadWrite(0x00);
    RegL = spiReadWrite(0x00);
    SS = 1;
}

#endif
