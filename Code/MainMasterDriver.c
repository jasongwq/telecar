#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "lt8910.h"
#include "usart.h"
#include "sys_os.h"
void tm0_isr() interrupt 1 using 1
{
    EA = 0;
  //  fTimer1ms = 1;
	UpdateTimers();
    EA = 1;
    
}
void spiReadreg(unsigned char reg)
{
    SS = 0;
    spiReadWrite(READ | reg);
    RegH = spiReadWrite(0x00);
    RegL = spiReadWrite(0x00);
    SS = 1;
}

sbit    OutR    = P3 ^ 1;           //output
sbit    OutL    = P3 ^ 3;           //output
sbit    OutF    = P3 ^ 5;           //output
sbit    OutB    = P3 ^ 6;           //input
sbit    PWM     = P1 ^ 1;           //input

char ControlCommand = -1;
#define Left              0
#define Right             1
#define Stop              2
#define Skid              3
#define RemoteControlRunH 4
#define RemoteControlRunM 5
#define RemoteControlRunL 6
#define RemoteControlBack 7
#define ManualControlRunH 8
#define ManualControlRunM 9
#define ManualControlRunL 10
#define ManualControlBack 11
int TimeCount = 0;
void PwmOut(char time, char highttime)
{
    time = 0;
    highttime = 0;
}
char TaskControl(void)
{
    _SS
    while (1)
    {
        WaitX(20);
        switch (ControlCommand)
        {
        case Left             : OutL = 1; WaitX(2); OutL = 0;  break;
        case Right            : OutR = 1; WaitX(2); OutR = 0; break;
        case Stop             : PwmOut(0, 82); WaitX(2); OutF = 0; break;
        case Skid             : PwmOut(0, 82); OutF = 0; break;
        case ManualControlRunL:
        case RemoteControlRunL: OutB = 0; OutF = 1; PwmOut(1, 22); break;
        case RemoteControlRunM: OutB = 0; OutF = 1; PwmOut(2, 32); break;
        case RemoteControlRunH: OutB = 0; OutF = 1; PwmOut(3, 82); break;
        case RemoteControlBack: OutF = 0; OutB = 1; PwmOut(2, 26); break;
        case ManualControlRunM: OutB = 0; OutF = 1; PwmOut(3, 32); break;
        case ManualControlRunH: OutB = 0; OutF = 1; PwmOut(5, 82); break;
        case ManualControlBack: OutF = 0; OutB = 1; PwmOut(3, 26); break;
        }
    }
    _EE
}
char TaskRf(void)
{
    _SS
    RESET_N = 0;
    WaitX(100);
    RESET_N = 1;
    WaitX(200);
    InitLT8900();
    WaitX(200);
    spiWriteReg(7, 0x01, 0x00);
    WaitX(2);
    spiWriteReg(7, 0x00, 0x30);
    WaitX(3);
    spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
    spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
    WaitX(5);
#if 1==DEBUGLT8910
    debuglt8910();
#endif
    while (1)
    {
			  //WaitX(40);
			//SendUart(RegL);
//        spiReadreg(48);
//        if (0x40 == (RegL & 0x40))
//        {
					if(1==PKT)
					{
            RegH = 0;
            spiReadreg(50);
            if (0x01 == RegH)
            {
                SendUart(RegL);
                spiWriteReg(7, 0x00, 0x30);
                WaitX(6);
                spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
//							  WaitX(3);
                spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
							WaitX(10);
            }
        }
    }
    _EE
}
void main(void)
{
    P_SW1 |= 0x80; //P_SW1 0x80 USART 在 RX P1.6 TX P1.7
    UartInit();

    SendUart(0x11);
    Timer0Init();
    for (;;)
    {
        RunTaskA(TaskRf, 0);
        RunTaskA(TaskControl, 1);
			;;;;;
			//SendUart(RegL);
    }
}
