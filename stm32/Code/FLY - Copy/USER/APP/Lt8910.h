#ifndef _LT8910_H
#define _LT8910_H
#include "spi51.h"
#include "minos_delay.h"
#include "gpio.h"

#ifdef __cplusplus
       extern "C" {
#endif

#define Delay10us() delay_us(10)
volatile unsigned char RegH, RegL;

#define LT8910_SS_Init   PB12_OUT
#define	LT8910_SS 		PBout(12)       		//output
#define	RESET_N_Init	PA0_OUT  			//output
#define	RESET_N	      PAout(0)  			//output
#define SPI0_SCK_Init   PB13_OUT
#define Set_SPI0_SCK    Set_B13
#define Clr_SPI0_SCK    Clr_B13
#define SPI0_MISO_Init  PB14_In
#define MISO_SPI0       B14
#define SPI0_MOSI_Init  PB15_OUT
#define Set_SPI0_MOSI   Set_B15
#define Clr_SPI0_MOSI   Clr_B15
u8 SPI0_ReadWriteByte(u8 TxData)                                        //SPI读写数据函数
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
			if(TxData & 0x80)
        {
            Set_SPI0_MOSI
        }
        else
        {
            Clr_SPI0_MOSI
        }

        TxData = (TxData << 1);                    // shift next bit into MSB..
        Set_SPI0_SCK                             // Set SCK high..
        TxData |= MISO_SPI0;                     // capture current MISO bit
        Clr_SPI0_SCK                             // ..then set SCK low again
    }
    return (TxData);                             // return read UINT8
}void SPI0_Init(void)
{
    SPI0_SCK_Init
    SPI0_MISO_Init
    SPI0_MOSI_Init
    SPI0_ReadWriteByte(0xff);                                            //启动传输
}

//unsigned char SPI0_ReadWriteByte(unsigned char Byte)
//{
//    unsigned char i;

//   // EA = 0;
//    for (i = 0; i < 8; i++)
//    {
//        MOSI = (Byte & 0x80);
//        SCLK = 1;
//        Delay10us();
//        Byte <<= 1;
//        Byte |= MISO;
//        SCLK = 0;
//        Delay10us();
//    }
//  //  EA = 1;

//    return (Byte);
//}

void spiWriteReg(unsigned char reg, unsigned char byteH, unsigned char byteL)
{
    LT8910_SS = 0;
    SPI0_ReadWriteByte(WRITE & reg);
    SPI0_ReadWriteByte(byteH);
    SPI0_ReadWriteByte(byteL);
    LT8910_SS = 1;
}
void spiReadreg(unsigned char reg)
{
    LT8910_SS = 0;
    SPI0_ReadWriteByte(READ | reg);
    RegH = SPI0_ReadWriteByte(0x00);
    RegL = SPI0_ReadWriteByte(0x00);
    LT8910_SS = 1;
}
#ifdef __cplusplus
        }
#endif
#endif
