#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "lt8910.h"
#include "usart.h"
#include "sys_os.h"
void tm0_isr() interrupt 1 using 1
{
    EA = 0;
    fTimer1ms = 1;
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
int a;
sbit    OutR    = P3 ^ 1;           //output
sbit    OutL    = P3 ^ 3;           //output
sbit    OutF    = P3 ^ 5;           //output
sbit    OutB    = P3 ^ 6;           //input
sbit    PWM     = P1 ^ 1;           //input
sbit 		
char Speed = 0;
char ControlCommand = -1;
#define Left              3
#define Right             2
#define Stop              4
#define Skid              1
#define RemoteControlRunH 0x27
#define RemoteControlRunM 0x17
#define RemoteControlRunL 7
#define RemoteControlBack 6
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
		P3M0=0x6A;
    while (1)
    {
        WaitX(1);
			if()
        if (ControlCommand == Left)
        {
            OutL = 1;
            WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
            OutL = 0;
					SendUart(ControlCommand);
        }
        else if (ControlCommand == Right)
        {
            OutR = 1;
            WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
            OutR = 0;
            SendUart(ControlCommand);
        }
        else if (ControlCommand == Stop)
        {
            PwmOut(0, 82);
            WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
					  WaitX(200);
            OutF = 0;
        }
        else if (ControlCommand == Skid)
        {
            PwmOut(0, 82);
            OutF = 0;
        }
        else if ((ControlCommand == ManualControlRunL) || (ControlCommand == RemoteControlRunL))
        {
            OutB = 0;
            OutF = 1;
            PwmOut(1, 22);
        }
        else if (ControlCommand == RemoteControlRunM)
        {
            OutB = 0;
            OutF = 1;
            PwmOut(2, 32);
        }
        else if (ControlCommand == RemoteControlRunH)
        {
            OutB = 0;
            OutF = 1;
            PwmOut(3, 82);
        }
        else if (ControlCommand == RemoteControlBack)
        {
            OutF = 0;
            OutB = 1;
            PwmOut(2, 26);
        }
        else if (ControlCommand == ManualControlRunM)
        {
            OutB = 0;
            OutF = 1;
            PwmOut(3, 32);
        }
        else if (ControlCommand == ManualControlRunH)
        {
            OutB = 0;
            OutF = 1;
            PwmOut(5, 82);
        }
        else if (ControlCommand == ManualControlBack)
        {
            OutF = 0;
            OutB = 1;
            PwmOut(3, 26);
        }
        ControlCommand = -1;
    }
    _EE
}
char TaskRf(void)
{
    _SS
    InitLT8900();
    spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
    spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
    delayMs(5);
    while (1)
    {
        WaitX(2);
        spiReadreg(48);
        if (0x40 == (RegL & 0x40))
        {
            spiReadreg(48);
            if (0x00 == (RegH & 0x80))
            {
                spiReadreg(50);
                if (0x01 == RegH)
                {
                    if (RegL < 0x28)ControlCommand = RegL;
                }
            }
            spiWriteReg(52, 0x80, 0x80);            // 清接收缓存区
            spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
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
    }
}
