#include "STC15W401.h"
#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "lt8910.h"
#include "usart.h"
#include "sys_os.h"
#include "pca.h"

void InitLT8900(void)
{
    RESET_N = 0;
    delayMs(100);
    RESET_N = 1;
    delayMs(200);
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
    spiWriteReg(13, 0x48, 0xbd);//14
    spiWriteReg(22, 0x00, 0xff);
    spiWriteReg(23, 0x80, 0x05);
    spiWriteReg(24, 0x00, 0x67);
    spiWriteReg(25, 0x16, 0x59);
    spiWriteReg(26, 0x19, 0xe0);
    spiWriteReg(27, 0x13, 0x00);
    spiWriteReg(28, 0x18, 0x00);//7
    spiWriteReg(32, 0x50, 0x00);
    spiWriteReg(33, 0x3f, 0xc7);
    spiWriteReg(34, 0x20, 0x00);
    spiWriteReg(35, 0x03, 0x00);
    spiWriteReg(36, 0x03, 0x80);
    spiWriteReg(37, 0x03, 0x80);
    spiWriteReg(38, 0x5a, 0x5a);
    spiWriteReg(39, 0x03, 0x80);
    spiWriteReg(40, 0x44, 0x02);
    spiWriteReg(41, 0xB0, 0x00);  //crc on scramble off ,1st byte packet length ,auto ack off
    spiWriteReg(42, 0xfd, 0xb0);  //
    spiWriteReg(43, 0x00, 0x0f);
    spiWriteReg(44, 0x01, 0x00);
    spiWriteReg(45, 0x01, 0x52);//14

    spiWriteReg(50, 0x00, 0x00);
}

volatile unsigned char  lastpwm0, pwm0 = 0xff - 36;
u16 i;
u32 tmp;
int TimeCount = 0;
int a;
char Speed = 0;
char ControlCommand = -1;
#define ID_ADDR_RAM 0xF1                //STC104W系列ID号的存放在RAM区的地址
unsigned char *pIdRam = 0xF1;

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
#define ProofreadingFrequency 0xf4


#define PWMOUT(time,highttime) \
    ControlCommand = -1;\
    lastpwm0 = pwm0;\
    for (i = 0; i < (time); i++){\
        if ((highttime) > pwm0){tmp = (u32)(highttime) * i / (time) + lastpwm0; if (tmp > (highttime)) break;else pwm0=tmp;}\
        else if ((highttime) < pwm0){tmp = lastpwm0 - (u32)(255-(highttime)) * i / (time); if (tmp < (highttime)) break;else pwm0=tmp;}\
        else break;\
        CCAP0H = 0xff - pwm0; WaitX(1);\
    } pwm0 = (highttime); CCAP0H = 0xff - pwm0

#define PWMOUTHIGH(time) \
    ControlCommand = -1;\
    lastpwm0 = pwm0;\
    for (i = 0; i < time; i++){\
        tmp = (u32)(255) * i / time + lastpwm0; if (tmp > 255)break; else pwm0 = tmp;\
        CCAP0H = 0xff - pwm0;WaitX(1);\
    } pwm0 = 255; CCAP0H = 0xff - pwm0

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
            pwm0 = 255; CCAP0H = 0xff - pwm0;
            OutF = 0;
        }
        else if ((ControlCommand == ManualControlRunL) || (ControlCommand == RemoteControlRunL))
        {
            OutB = 0; OutF = 1;
            PWMOUT(1000, 66); //PwmOut(1, 22 * 3);
        }
        else if (ControlCommand == RemoteControlRunM)
        {
            OutB = 0; OutF = 1;
            PWMOUT(2000, 96); //PwmOut(2, 32 * 3);
        }
        else if (ControlCommand == RemoteControlRunH)
        {
            OutB = 0; OutF = 1;
            PWMOUTHIGH(3000);//PwmOut(3, 255);
        }
        else if (ControlCommand == RemoteControlBack)
        {
            OutF = 0; OutB = 1;
            PWMOUT(2000, 78);//PwmOut(2, 26 * 3);
        }
        else if (ControlCommand == ManualControlRunM)
        {
            OutB = 0; OutF = 1;
            PWMOUT(3000, 96);//PwmOut(3, 32 * 3);
        }
        else if (ControlCommand == ManualControlRunH)
        {
            OutB = 0; OutF = 1;
            PWMOUTHIGH(5000);//PwmOut(5, 255);
        }
        else if (ControlCommand == ManualControlBack)
        {
            OutF = 0; OutB = 1;
            pwm0 = 36;
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
            OutL = 1; WaitX(1000); OutL = 0;
        }
        else if (ControlCommand == Right)
        {
            ControlCommand = -1;
            OutR = 1; WaitX(1000); OutR = 0;
        }
        else if (ControlCommand == Stop)
        {
            ControlCommand = -1;
            pwm0 = 255; CCAP0H = 0xff - pwm0;
            WaitX(1000);
            OutF = 0;
        }
        else if (ControlCommand == Skid || ControlCommand == RemoteControlRunH || ControlCommand == RemoteControlRunM || ControlCommand == RemoteControlRunL || ControlCommand == RemoteControlBack || ControlCommand == ManualControlRunH || ControlCommand == ManualControlRunM || ControlCommand == ManualControlRunL || ControlCommand == ManualControlBack);
        else ControlCommand = -1;
    }
    _EE
}
int TaskRf(void)
{
    _SS
    InitLT8900();
    spiWriteReg(7, 0x00, 0x30);
    delayMs(3);
    spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
    spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
    delayMs(5);
    for (i = 0; i < 10; i++)
    {
        spiWriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
        spiWriteReg(52, 0x80, 0x00);            // 清空发送缓存区
        // 发送1个字节
        spiWriteReg(50, 5, *(pIdRam + 6));
        spiWriteReg(50, *(pIdRam + 0), *(pIdRam + 1));
        spiWriteReg(50, *(pIdRam + 2), *(pIdRam + 3));
        spiWriteReg(7, 0x01, 0x30);             // 允许发射使能
        WaitX(20);
    }

    spiWriteReg(36, *(pIdRam + 0), *(pIdRam + 1));
    spiWriteReg(38, *(pIdRam + 2), *(pIdRam + 3));
    spiWriteReg(39, 0xbd, *(pIdRam + 6));
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
                    if (RegL < 0x48)ControlCommand = RegL;
                    if ((RegL & 0x0f) == 0x07 || (RegL & 0x0f) == 0x05)
                    {
                        Speed = RegL >> 4;
                        if ((RegL & 0x0f) == 0x05)
                        {
                            ControlCommand = ((RegL & 0xf0) | 0x07);
                        }
                    }
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
    for (i = 0; i < 7; i++)SendUart(*pIdRam++);
    pIdRam = ID_ADDR_RAM;
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
