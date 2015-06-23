#include "STC15W401.h"
#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "eeprom.h"
volatile int t = 0;
bit t1 = 0;
bit Realse = 0;
char Speed = 0;
char LastKey = 0;
u16 KeyCount = 0;
u16 KeyCount1 = 0;

void tm0_isr() interrupt 1 using 1
{
    EA = 0;
    fTimer1ms = 1;
    t++; if (t > 20)
    {
        t = 0;
        t1 = 1;
    }
    EA = 1;
}

#define DEBUGUSART 0
#if 1==DEBUGUSART
#include "usart.h"
#endif
#define DEBUGLT8910 0
#include "lt8910.h"
void InitLT8900(void)
{
    u8 EepromTmp[3 * 34];
    char i;
    EEPROM_read_n(0x0200, EepromTmp, 3 * 34);
    RESET_N = 0;
    delayMs(100);
    RESET_N = 1;
    delayMs(200);
    SCLK = 0;
    for (i = 0; i < 34; i++)spiWriteReg(EepromTmp[i], EepromTmp[i + i + 34], EepromTmp[i + i + 34+1]);
}
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

#define Left              0x03
#define Right             0x02
#define Stop              0x0A
#define Skid              0x01
#define RemoteControlRunH 0x27
#define RemoteControlRunM 0x17
#define RemoteControlRunL 0x07
#define RemoteControlBack 0x06
#define ManualControlRunH 0x28
#define ManualControlRunM 0x18
#define ManualControlRunL 0x08
#define ManualControlBack 0x09
#define ProofreadingFrequency 0x04
void LED(u8 Stata)
{
    LEDH = (Stata >> 6);
    LEDM = (Stata >> 5) & 0x01;
    LEDL = (Stata >> 4) & 0x01;
}
char KeyScan(void)
{
    if      (0 == KeyT)return (5 | ((Speed > 1 ? (3 - Speed) : (Speed + 1)) << 4));//调速
    else if (0 == KeyR)return Right;//右
    else if (0 == KeyL)return Left;//左
    else if (0 == KeyD)return ProofreadingFrequency;//对频
    else if (0 == KeyS)return Skid;//刹车
    else if (0 == KeyB)return RemoteControlBack;//后
    else if (0 == KeyF)return (7 + (Speed << 4));//前
    else return 0;
}
u8 AddressFrequency[5];
volatile char Key;
void exint0() interrupt 0
{
}
void SetLT9010Address(void)
{
    spiWriteReg(39, 0xbd, AddressFrequency[0]);
    spiWriteReg(36, AddressFrequency[1], AddressFrequency[2]);
    spiWriteReg(38, AddressFrequency[3], AddressFrequency[4]);
}
void FunProofreadingFrequency(void)
{
    KeyCount++; //SendUart(KeyCount);
    if (KeyCount > 100)
    {
        spiWriteReg(36, 0x03, 0x80);
        spiWriteReg(38, 0x5a, 0x5a);
        spiWriteReg(39, 0x03, 0x80);

        spiWriteReg(7, 0x00, 0x30);
        delayMs(3);
        spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
        spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
        delayMs(50);
        LEDF = 0;
        for (;;)
        {
            spiReadreg(48);
            if (0x40 == (RegL & 0x40))
            {
                spiReadreg(48);
                if (0x00 == (RegH & 0x80))
                {
                    spiReadreg(50);
                    if (0x05 == RegH)
                    {
                        AddressFrequency[0] = RegL;
                        spiReadreg(50); AddressFrequency[1] = RegH; AddressFrequency[2] = RegL;
                        spiReadreg(50); AddressFrequency[3] = RegH; AddressFrequency[4] = RegL;
                        SetLT9010Address();
                        EEPROM_SectorErase(0x0000);
                        EEPROM_write_n(0x0000, AddressFrequency, 5);
                        LEDF = 1;
                        break;
                    }
                }
                spiWriteReg(52, 0x80, 0x80);            // 清接收缓存区
                spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
            }
            delayMs(2);
        }
    }
}
void RfSend(u8 Data)
{
    spiWriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
    spiWriteReg(52, 0x80, 0x00);            // 清空发送缓存区
    // 发送1个字节
    spiWriteReg(50, 1, Data );
    spiWriteReg(7, 0x01, 0x30);             // 允许发射使能
}
void main(void)
{
#if 1==DEBUGUSART
    UartInit();
    SendUart(0x11);
#endif
    Timer0Init();
    InitLT8900();
    EEPROM_read_n(0x0000, AddressFrequency, 5);
    SetLT9010Address();
    KeyT    = 1;
    KeyR    = 1;
    KeyL    = 1;
    KeyD    = 1;
    KeyS    = 1;
    KeyB    = 1;
    KeyF    = 1;

    LEDL    = 0;
    LEDH    = 1;
    LEDM    = 1;
    LEDF    = 1;

#if 1==DEBUGLT8910
    debuglt8910();
#endif
    IT0 = 1;
    EX0 = 1;
    EA = 1;
    while (1)
    {
        Key = KeyScan();
        if (t1 == 1)
        {
            t1 = 0;
            if (0 != Key)
            {
                LastKey = Key;
                Realse = 1;
                KeyCount1 = 0;
                if (Key == ProofreadingFrequency)
                {
                    FunProofreadingFrequency();
                }
                RfSend(((Key & 0xf0) == 0x30) ? (Key & 0xdf) : Key);

            }
            else if (Realse)
            {
                Realse = 0;
                KeyCount = 0;
                if ((LastKey & 0x0f) == 5)
                {
                    switch (++Speed)
                    {//低电平亮
                    case 4: Speed = 0; LED(0x60); break;//L亮
                    case 1: LED(0x40); break;//M亮
                    case 2: LED(0x00); break;//H亮
                    case 3: LED(0x40); break;//M亮
                    default: Speed = 0; break;
                    }
                }
                RfSend(0xff);
            }
            else
            {
                if (KeyCount1++ > 700)
                {
                    u8 SleepSave;
                    KeyCount1 = 0;
                    SleepSave = (SleepSave << 1) | LEDF;
                    SleepSave = (SleepSave << 1) | LEDH;
                    SleepSave = (SleepSave << 1) | LEDM;
                    SleepSave = (SleepSave << 1) | LEDL;
                    SleepSave = (SleepSave << 4);
                    LEDF = 1;
                    LED(0x70);
                    PCON = 0x02;
                    _nop_();
                    _nop_();
                    _nop_();
                    _nop_();
                    LEDF = (SleepSave >> 7) & 0x01;
                    LED(SleepSave);
                }
            }
        }
    }
}
