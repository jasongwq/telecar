#ifndef _LT8910_H
#define _LT8910_H

#define DEBUGLT8910 0
unsigned char RegH, RegL;
unsigned char Reg2H, Reg2L;

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
#if 1==DEBUGLT8910
void debuglt8910(void)
{
    spiReadreg(0); SendUart(RegL); SendUart(RegH);
    spiReadreg(1 ); SendUart(RegL); SendUart(RegH);
    spiReadreg(2 ); SendUart(RegL); SendUart(RegH);
    spiReadreg(4 ); SendUart(RegL); SendUart(RegH);
    spiReadreg(5 ); SendUart(RegL); SendUart(RegH);
    spiReadreg(7 ); SendUart(RegL); SendUart(RegH);
    spiReadreg(8 ); SendUart(RegL); SendUart(RegH);
    spiReadreg(9 ); SendUart(RegL); SendUart(RegH);
    spiReadreg(10); SendUart(RegL); SendUart(RegH);
    spiReadreg(11); SendUart(RegL); SendUart(RegH);
    spiReadreg(12); SendUart(RegL); SendUart(RegH);
    spiReadreg(13); SendUart(RegL); SendUart(RegH);
    spiReadreg(22); SendUart(RegL); SendUart(RegH);
    spiReadreg(23); SendUart(RegL); SendUart(RegH);
    spiReadreg(24); SendUart(RegL); SendUart(RegH);
    spiReadreg(25); SendUart(RegL); SendUart(RegH);
    spiReadreg(26); SendUart(RegL); SendUart(RegH);
    spiReadreg(27); SendUart(RegL); SendUart(RegH);
    spiReadreg(28); SendUart(RegL); SendUart(RegH);
    spiReadreg(32); SendUart(RegL); SendUart(RegH);
    spiReadreg(33); SendUart(RegL); SendUart(RegH);
    spiReadreg(34); SendUart(RegL); SendUart(RegH);
    spiReadreg(35); SendUart(RegL); SendUart(RegH);
    spiReadreg(36); SendUart(RegL); SendUart(RegH);
    spiReadreg(37); SendUart(RegL); SendUart(RegH);
    spiReadreg(38); SendUart(RegL); SendUart(RegH);
    spiReadreg(39); SendUart(RegL); SendUart(RegH);
    spiReadreg(40); SendUart(RegL); SendUart(RegH);
    spiReadreg(41); SendUart(RegL); SendUart(RegH);
    spiReadreg(42); SendUart(RegL); SendUart(RegH);
    spiReadreg(43); SendUart(RegL); SendUart(RegH);
    spiReadreg(50); SendUart(RegL); SendUart(RegH);    
}
#endif
void InitLT8900(void)
{

    SCLK = 0;

    spiWriteReg(0, 0x6f, 0xe0);
    spiWriteReg(1, 0x56, 0x81);
    spiWriteReg(2, 0x66, 0x17);
    spiWriteReg(4, 0x9c, 0xc9);
    spiWriteReg(5, 0x66, 0x37);
    spiWriteReg(7, 0x00, 0x00);
    spiWriteReg(8, 0x6c, 0x90);
    spiWriteReg(9, 0x48, 0x00);             // 5.5dBm
    spiWriteReg(10, 0x7f, 0xfd);
    spiWriteReg(11, 0x00, 0x08);
    spiWriteReg(12, 0x00, 0x00);
    spiWriteReg(13, 0x48, 0xbd);

    spiWriteReg(22, 0x00, 0xff);
    spiWriteReg(23, 0x80, 0x05);
    spiWriteReg(24, 0x00, 0x67);
    spiWriteReg(25, 0x16, 0x59);
    spiWriteReg(26, 0x19, 0xe0);
    spiWriteReg(27, 0x13, 0x00);
    spiWriteReg(28, 0x18, 0x00);

    spiWriteReg(32, 0x48, 0x00);
    spiWriteReg(33, 0x3f, 0xc7);
    spiWriteReg(34, 0x20, 0x00);
    spiWriteReg(35, 0x03, 0x00);
    spiWriteReg(36, 0x03, 0x80);
    spiWriteReg(37, 0x03, 0x80);
//    spiWriteReg(38, 0x5a, 0x5a);
    spiWriteReg(39, 0x03, 0x80);
    spiWriteReg(40, 0x44, 0x02);
    spiWriteReg(41, 0xB0, 0x00);  //crc on scramble off ,1st byte packet length ,auto ack off
    spiWriteReg(42, 0xfd, 0xb0);  //
    spiWriteReg(43, 0x00, 0x0f);
    spiWriteReg(44, 0x01, 0x00);
    spiWriteReg(45, 0x01, 0x52);

    spiWriteReg(50, 0x00, 0x00);
}

#endif