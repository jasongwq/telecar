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
    u8 EepromTmp[3 * 34] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x16, 0x17, 0x18, 0x19, 0x1a,
                            0x1b, 0x1c, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x32,
                            0x6f, 0xe0, 0x56, 0x81, 0x66, 0x17, 0x9c, 0xc9, 0x66, 0x37, 0x00, 0x00, 0x6c, 0x90, 0x48, 0x00, 0x7f,
                            0xfd, 0x00, 0x08, 0x00, 0x00, 0x48, 0xbd, 0x00, 0xff, 0x80, 0x05, 0x00, 0x67, 0x16, 0x59, 0x19, 0xe0,
                            0x13, 0x00, 0x18, 0x00, 0x48, 0x20, 0x3f, 0xc7, 0x20, 0x00, 0x03, 0x00, 0x03, 0x80, 0x03, 0x80, 0x03,
                            0x80, 0x03, 0x80, 0x44, 0x02, 0xb0, 0x00, 0xfd, 0xb0, 0x00, 0x0f, 0x01, 0x00, 0x04, 0x80, 0x00, 0x00
                           };
    char i;
    RESET_N = 0;
    delayMs(100);
    RESET_N = 1;
    delayMs(200);
    SCLK = 0;
    for (i = 0; i < 34; i++)spiWriteReg(EepromTmp[i], EepromTmp[i + i + 34], EepromTmp[i + i + 34 + 1]);
}
#define ID_ADDR_RAM 0xF1

volatile u8 lastpwm0, pwm0 = 0xff - 36;
u8 *pIdRam = ID_ADDR_RAM;
u8 Speed     = 0;
s8 ControlCommand   = -1;
u16 i   = 0;
u32 tmp = 0;
static u8 Runing = 0;
static u8 Turning = 0;


sbit    OutR = P3 ^ 1;//output
sbit    OutL = P3 ^ 3;//output
sbit    OutF = P3 ^ 5;//output
sbit    OutB = P3 ^ 6;//output
sbit    PWM  = P1 ^ 1;//output
sbit    INMF = P3 ^ 7;//input
sbit    INMB = P1 ^ 0;//input

void tm0_isr() interrupt 1 using 1
{
    EA = 0;
    fTimer1ms = 1;
    UpdateTimers();
    EA = 1;
}

#define Left                 0x10//43
#define Right                0x18//43
#define Stop                 0x84
#define Skid                 0x80//70
#define RemoteControlSpeed   0x20//5
#define RemoteControlRun     0x04//2
#define RemoteControlRunH    0x06//210
#define RemoteControlRunM    0x05//210
#define RemoteControlRunL    0x04//210
#define RemoteControlBack    0x81//70
#define ManualControlRun     0x40//6
#define ManualControlRunH    0x42//610
#define ManualControlRunM    0x41//610
#define ManualControlRunL    0x40//610
#define ManualControlBack    0x83//710
#define ProofreadingFrequency 0x82//710


#define PWMOUT(time,highttime) \
    Runing=1;\
    ControlCommand = -1;\
    lastpwm0 = pwm0;\
    for (i = 0; i < (time); i++){\
        if ((highttime) > pwm0){tmp = (u32)(highttime) * i / (time) + lastpwm0; if (tmp > (highttime)) break;else pwm0=tmp;}\
        else if ((highttime) < pwm0){tmp = lastpwm0 - (u32)(255-(highttime)) * i / (time); if (tmp < (highttime)) break;else pwm0=tmp;}\
        else break;\
        CCAP0H = 0xff - pwm0; WaitX(1);\
    } pwm0 = (highttime); CCAP0H = 0xff - pwm0

#define PWMOUTHIGH(time) \
    Runing=1;\
    ControlCommand = -1;\
    lastpwm0 = pwm0;\
    for (i = 0; i < time; i++){\
        tmp = (u32)(255) * i / time + lastpwm0; if (tmp > 255)break; else pwm0 = tmp;\
        CCAP0H = 0xff - pwm0;WaitX(1);\
    } pwm0 = 255; CCAP0H = 0xff - pwm0

char TaskControl(void)
{
    static s16 DirectionCount = 0;
    _SS
    while (1)
    {
        WaitX(1);
        if (0 == INMF)ControlCommand = ManualControlRun | Speed;
        if (0 == INMB)ControlCommand = ManualControlBack;
        if (ControlCommand == Skid)
        {
            pwm0 = 255; CCAP0H = 0xff - pwm0;
            OutF = 0; OutB = 0;
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
        if (ControlCommand == Left)
        {
            Turning = 1;
            ControlCommand = -1;
            if (DirectionCount < 1000) {OutL = 1; DirectionCount++;} //WaitX(1000); OutL = 0;
        }
        else if (ControlCommand == Right)
        {
            Turning = 1;
            ControlCommand = -1;
            if (DirectionCount > -1000)
            {OutR = 1; DirectionCount--;} //WaitX(1000); OutR = 0;
        }
        if (Turning)
        {
            if (Turning++ > 40)
            {
                OutR = 0;
                OutL = 0;
                Turning = 0;
            }
        }
        if (Runing)
        {
            if (Runing++ > 40)
            {
                ControlCommand = Stop;
                Runing = 0;
            }
        }

    }
    _EE
}
int TaskControl2(void)
{
    _SS
    while (1)
    {
        WaitX(1);
        if (ControlCommand == Stop)
        {
            pwm0 = 255; CCAP0H = 0xff - pwm0;
            WaitX(1000);
            if (Runing == 0) {OutF = 0; OutB = 0; ControlCommand = -1;}
        }
    }
    _EE
}
#define FIFONUM 4
int TaskRf(void)
{   static u8 RfFifo[FIFONUM];
    _SS
    InitLT8900();
    spiWriteReg(7, 0x00, 0x30);
    delayMs(3);
    spiWriteReg(52, 0x00, 0x80);
    spiWriteReg(7, 0x00, 0xB0);
    delayMs(5);
    for (i = 0; i < 10; i++)
    {
        spiWriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
        spiWriteReg(52, 0x80, 0x00);

        spiWriteReg(50, 5, *(pIdRam + 6));
        spiWriteReg(50, *(pIdRam + 0), *(pIdRam + 1));
        spiWriteReg(50, *(pIdRam + 2), *(pIdRam + 3));
        spiWriteReg(7, 0x01, 0x30);
        WaitX(20);
    }

    spiWriteReg(36, *(pIdRam + 0), *(pIdRam + 1));
    spiWriteReg(38, *(pIdRam + 2), *(pIdRam + 3));
    spiWriteReg(39, 0xbd, *(pIdRam + 6));
    spiWriteReg(52, 0x00, 0x80);
    spiWriteReg(7, 0x00, 0xB0);
    delayMs(5);
    while (1)
    {
        static unsigned char lasti;
        u8 Data;
        u8 test;
        WaitX(2);
        spiReadreg(48);
        if (0x40 == (RegL & 0x40))
        {
            spiReadreg(48);
            if (0x00 == (RegH & 0x80))
            {
                spiReadreg(50); RfFifo[0] = RegH; RfFifo[1] = RegL;
                if ((FIFONUM - 1) == RfFifo[0])
                {
                    spiReadreg(50);
                    RfFifo[2]  = RegH; RfFifo[3] = RegL;
                    test = (u8)((u8)RfFifo[1] - (u8)lasti) > 10 ? (lasti - RfFifo[1]) : (RfFifo[1] - lasti);
                    if (1 == test)
                    {
                        Data = RfFifo[3];
                    }
                    else
                    {
                        Data = RfFifo[2];
                        ControlCommand = Data ;
                        if ((Data  & 0x84) == 0x04)
                        {
                            Speed = (Data & 0x03);
                        }
                        SendUart(ControlCommand);
                        WaitX(2);
                        Data = RfFifo[3];
                    }
                    lasti = RfFifo[1];
                    ControlCommand = Data ;
                    if ((Data  & 0x84) == 0x04)
                    {
                        Speed = (Data & 0x03);
                    }
                    SendUart(ControlCommand);
                }
            }
            spiWriteReg(7, 0x00, 0x30);
            spiWriteReg(52, 0x80, 0x80);
            spiWriteReg(7, 0x00, 0xB0);
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
    P_SW1 |= 0x80; //P_SW1 0x80 USART ÔÚ RX P1.6 TX P1.7
    UartInit();
    for (i = 0; i < 7; i++)SendUart(*pIdRam++);
    pIdRam = ID_ADDR_RAM;
    Timer0Init();
    INMF = 1;
    INMB = 1;
    for (;;)
    {
        RunTaskA(TaskControl, 0);
        RunTaskA(TaskControl2, 1);
        RunTaskA(TaskRf, 2);

    }
}
