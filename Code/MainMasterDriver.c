#include "STC15W401.h"
#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "lt8910.h"
#include "usart.h"
#include "sys_os.h"
#include "pca.h"
volatile unsigned char  lastpwm0, pwm0 = 0;
u16 i;
u32 tmp;

sbit    OutR    = P3 ^ 1;//output
sbit    OutL    = P3 ^ 3;//output
sbit    OutF    = P3 ^ 5;//output
sbit    OutB    = P3 ^ 6;//output
sbit    PWM     = P1 ^ 1;//output
sbit    INMF    = P3 ^ 7;//input
sbit    INMB    = P1 ^ 0;//input

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
char Speed = 0;
char ControlCommand = -1;
#define Left              3
#define Right             2
#define Stop              4
#define Skid              1
#define RemoteControlRunH 0x27
#define RemoteControlRunM 0x17
#define RemoteControlRunL 0x07
#define RemoteControlBack 6
#define ManualControlRunH 0x28
#define ManualControlRunM 0x18
#define ManualControlRunL 0x08
#define ManualControlBack 11
int TimeCount = 0;
// void PwmOut(char time, char highttime)
// {
//     if (time == 0)CCAP0H = 0xff - pwm0;
// }
#define PWMOUT(time,highttime) \
    lastpwm0 = pwm0;\
    for (i = 0; i < (time); i++){\
        if ((highttime) > pwm0){tmp = (u32)(highttime) * i / (time) + lastpwm0; if (tmp > (highttime)) break;else pwm0=tmp;}\
        else if ((highttime) < pwm0){tmp = lastpwm0 - (u32)(255-(highttime)) * i / (time); if (tmp < (highttime)) break;else pwm0=tmp;}\
        else break;\
        CCAP0H = 0xff - pwm0; WaitX(1);\
    } pwm0 = (highttime); CCAP0H = 0xff - pwm0

char TaskControl(void)
{
    _SS
    while (1)
    {
        WaitX(1);
        if (0 == INMF)ControlCommand = RemoteControlRunL | Speed;
        if (0 == INMB)ControlCommand = ManualControlBack | Speed;
        if (ControlCommand == Skid)
        {
            ControlCommand = -1;
            pwm0 = 255; CCAP0H = 0xff - pwm0;
            OutF = 0;
        }
        else if ((ControlCommand == ManualControlRunL) || (ControlCommand == RemoteControlRunL))
        {
            ControlCommand = -1;
            OutB = 0;
            OutF = 1;
            lastpwm0 = pwm0;
            PWMOUT(1000, 66); //PwmOut(1, 22 * 3);
        }
        else if (ControlCommand == RemoteControlRunM)
        {
            ControlCommand = -1;
            OutB = 0; OutF = 1;
            PWMOUT(2000, 96); //PwmOut(2, 32 * 3);
					SendUart(ControlCommand);
        }
        else if (ControlCommand == RemoteControlRunH)
        {
            SendUart(ControlCommand);
            ControlCommand = -1;
            OutB = 0;
            OutF = 1;
            lastpwm0 = pwm0;
            for (i = 0; i < 3000; i++)
            {
                tmp = (u32)(255) * i / 3000 + lastpwm0; if (tmp > 255)break; else pwm0 = tmp;
                CCAP0H = 0xff - pwm0; SendUart(tmp >> 8); WaitX(1);
            } pwm0 = 255; CCAP0H = 0xff - pwm0; //PwmOut(3, 255);
        }
        else if (ControlCommand == RemoteControlBack)
        {
            ControlCommand = -1;
            OutF = 0; OutB = 1;
            PWMOUT(2000, 78);//PwmOut(2, 26 * 3);
        }
        else if (ControlCommand == ManualControlRunM)
        {
            ControlCommand = -1;
            OutB = 0; OutF = 1;
            PWMOUT(3000, 96);//PwmOut(3, 32 * 3);
        }
        else if (ControlCommand == ManualControlRunH)
        {
            ControlCommand = -1;
            OutB = 0; OutF = 1;
            lastpwm0 = pwm0;
            lastpwm0 = pwm0;
            for (i = 0; i < 5000; i++)
            {
                tmp = (u32)(255) * i / 5000 + lastpwm0; if (tmp > 255)break; else pwm0 = tmp;
                CCAP0H = 0xff - pwm0; SendUart(tmp >> 8); SendUart(tmp); WaitX(1);
            } pwm0 = 255; CCAP0H = 0xff - pwm0;//PwmOut(5, 255);
        }
        else if (ControlCommand == ManualControlBack)
        {
            ControlCommand = -1;
            OutF = 0; OutB = 1;
					  pwm0=36;
            PWMOUT(3000, 78);//PwmOut(3, 26 * 3);
        }
        else if (ControlCommand == Left || ControlCommand == Right || ControlCommand == Stop);
        else ControlCommand = -1;
    }
    _EE
}
int TaskControl2(void)
{
    _SS
    while (1)
    {
        WaitX(1);
        if (ControlCommand == Left)
        {
            ControlCommand = -1;
            OutL = 1;
            WaitX(1000);
            OutL = 0;
            SendUart(ControlCommand);
        }
        else if (ControlCommand == Right)
        {
            ControlCommand = -1;
            OutR = 1;
            WaitX(1000);
            OutR = 0;
            SendUart(ControlCommand);
        }
        else if (ControlCommand == Stop)
        {
            ControlCommand = -1;
            //PwmOut(0, 255);
            pwm0 = 255; CCAP0H = 0xff - pwm0;
            WaitX(1000);
            OutF = 0;
        }

    }
    _EE
}
int TaskRf(void)
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
                    if (RegL & 0x0f == 0x07 || RegL & 0x0f == 0x05){Speed = RegL >> 4;}
										SendUart(ControlCommand);
										
                }
            }
            spiWriteReg(52, 0x80, 0x80);            // 清接收缓存区
            spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
        }
    }
    _EE
}
#include "pwm.h"

void main(void)
{
    P3M0 = 0x6A;
    OutR = 0;
    OutL = 0;
    OutF = 0;
    OutB = 0;

    PCA_config();
    CCAP0H = pwm0;

    P_SW1 |= 0x80; //P_SW1 0x80 USART 在 RX P1.6 TX P1.7
    UartInit();
    SendUart(0x11);
    Timer0Init();
    INMF = 1;
    INMB = 1;
    for (;;)
    {
        RunTaskA(TaskControl2, 0);
        RunTaskA(TaskRf, 2);
        RunTaskA(TaskControl, 1);

    }
}
