#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "lt8910.h"
#define DEBUGUSART 0
#if 1==DEBUGUSART
#include "usart.h"
#endif
sbit    KeyT    = P3 ^ 0;           //output
sbit    KeyR    = P3 ^ 1;           //output
sbit    KeyL    = P3 ^ 2;           //output
sbit    KeyD    = P3 ^ 3;           //input
sbit    KeyS    = P3 ^ 4;           //output
sbit    KeyB    = P1 ^ 0;           //input
sbit    KeyF    = P1 ^ 1;           //output

sbit    LEDF    = P3 ^ 7;           //output
sbit    LEDH    = P5 ^ 5;           //output
sbit    LEDM    = P5 ^ 4;           //output
sbit    LEDL    = P1 ^ 7;           //output

void tm0_isr() interrupt 1 using 1
{
    EA = 0;
    fTimer1ms = 1;
    EA = 1;
}
char KeyScan(void)
{
    if      (0 == KeyT)return 1;
    else if (0 == KeyR)return 2;
    else if (0 == KeyL)return 3;
    else if (0 == KeyD)return 4;
    else if (0 == KeyS)return 5;
    else if (0 == KeyB)return 6;
    else if (0 == KeyF)return 7;
    else return 0;
}
volatile char Key;
void main(void)
{
#if 1==DEBUGUSART
    UartInit();
    SendUart(0x11);
#endif
    Timer0Init();
    InitLT8900();
    KeyT    = 1;
    KeyR    = 1;
    KeyL    = 1;
    KeyD    = 1;
    KeyS    = 1;
    KeyB    = 1;
    KeyF    = 1;
    LEDH    = 0;
    //delayMs(10000);
    LEDM    = 0;
    //delayMs(10000);
    LEDL    = 0;
    //delayMs(10000);
    LEDF    = 0;
    //delayMs(10000);
    LEDH    = 1;
    //delayMs(10000);
    LEDM    = 1;
    //delayMs(10000);
    LEDL    = 1;
    //delayMs(10000);
    LEDF    = 1;
    spiWriteReg(7, 0x00, 0x30);
    delayMs(3);
    spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
    spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
    delayMs(5);
#if 1==DEBUGLT8910
    debuglt8910();
#endif
    while (1)
    {
        static int sendcnt = 0;
        Key = KeyScan();
        if (t1 == 1 && 0 != Key)
        {
            t1 = 0;
            spiWriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
            spiWriteReg(52, 0x80, 0x00);            // 清空发送缓存区
            // 发送1个字节
            spiWriteReg(50, 1, Key);
            sendcnt = (++sendcnt) % 11;
            spiWriteReg(7, 0x01, 0x30);             // 允许发射使能

#if 1==DEBUGUSART
            SendUart(sendcnt);
#endif
        }
    }
}
