#include "user.h"
#include "STC15W401.h"
#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "lt8910.h"
#include "usart.h"
#include "sys_os.h"
#include "pca.h"

#define CONTROL_RUNING_IDLE     200
#define CONTROL_TRUNING_IDLE    200
#define CONTROL_NORECIVING_IDLE 200

#define ID_ADDR_RAM 0xF1
#define PWMSETATOMIC(x) do{EA = 0;STRPWM.pwm0 = (x); CCAP0H = STRPWM.pwm0; EA = 1;}while(0);

struct {
	volatile double PwmStep;
	volatile double pwm0;
	volatile u16 PwmTime;
	volatile u8 TargetDutyfactor;
} STRPWM = {0, 0, 0, 0};

u8 *pIdRam = ID_ADDR_RAM;
struct {
	volatile u8 ControlCommand;
	volatile u8 StopCommand;
	volatile u8 Runing;
	volatile u8 Turning;
	volatile u8 NoReceiving;
	volatile u8 Speed;
	s16 RDirectionCount;
	s16 LDirectionCount;
} Control = { -1, 0, 0, 0, 0, 0, 0, 0};

u8 i1 = 0;
u8 i2 = 0;
u16 i                = 0;
volatile bit st     = 0;
volatile bit bLeft  = 0;
volatile bit bRight = 0;

sbit    OutR         = P3 ^ 1;//output
sbit    OutL         = P3 ^ 3;//output
sbit    OutF         = P3 ^ 5;//output
sbit    OutB         = P3 ^ 6;//output
sbit    PWM          = P1 ^ 1;//output
sbit    INMF         = P1 ^ 0;//input
sbit    INMB         = P3 ^ 7;//input

void InitLT8900(void)
{
	code u8 EepromTmp[3 * 34] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x16, 0x17, 0x18, 0x19, 0x1a,
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

void tm0_isr() interrupt 1 using 1
{
	EA = 0;
	fTimer1ms = 1;
	if (bRight)
		if (Control.RDirectionCount > -1200) {Control.LDirectionCount = 0; Control.RDirectionCount--; OutR = 1;}
		else bRight = 0;
	else OutR = 0;
	if (bLeft)
		if (Control.LDirectionCount <  1200) {Control.RDirectionCount = 0; Control.LDirectionCount++; OutL = 1;}
		else bLeft = 0;
	else OutL = 0;
	if (i1++ > 10)
	{
		i1 = 0;
		if (STRPWM.PwmTime > 1)
		{
			//SendUart(STRPWM.pwm0);
			STRPWM.PwmTime--;
			STRPWM.pwm0 += STRPWM.PwmStep;
			CCAP0H = STRPWM.pwm0;
		}
		else if (STRPWM.PwmTime == 1)
		{
			STRPWM.PwmTime--;
			STRPWM.pwm0 = STRPWM.TargetDutyfactor;
			CCAP0H = STRPWM.pwm0;
		}
	}
	UpdateTimers();
	EA = 1;
}

void PwmCurve(u16 time, u8 TargetDutyfactor)
{
	s16 Dvalue;
	Control.Runing = 1;
	STRPWM.TargetDutyfactor = TargetDutyfactor;
	EA      = 0;
	if (STRPWM.pwm0 < 36)STRPWM.pwm0 = 36;
	Dvalue  = TargetDutyfactor - STRPWM.pwm0;
	STRPWM.PwmStep = (TargetDutyfactor - 36) / (double)time;
	STRPWM.PwmTime = Dvalue / STRPWM.PwmStep;
	if (Dvalue < 0)STRPWM.PwmStep = -STRPWM.PwmStep;
	EA = 1;
}
char TaskControl(void)
{
	_SS
	while (1)
	{
		WaitX(1);
		if ((Control.ControlCommand & MaskLeft) == Left)
		{
			Control.Turning = 1;
			Control.ControlCommand &= 0xe7;
			EA = 0; bLeft = 1;bRight = 0; EA = 1;
		}
		else if ((Control.ControlCommand & MaskRight) == Right)
		{
			Control.Turning = 1;
			Control.ControlCommand &= 0xe7;
			EA = 0; bLeft = 0;bRight = 1; EA = 1;
		}
		if (Control.ControlCommand == Skid)
		{	Control.ControlCommand = -1;
			//PWMSETATOMIC(0)
			OutF = 0; OutB = 0;
		}
		else if (Control.NoReceiving > CONTROL_NORECIVING_IDLE)
		{
			if (0 == INMF) {Control.ControlCommand = ManualControlRun | Control.Speed;}
			else if (0 == INMB) {Control.ControlCommand = ManualControlBack;}
		}
		if (((Control.ControlCommand & MaskRemoteControlRunL) == ManualControlRunL) || (Control.ControlCommand == RemoteControlRunL))
		{	if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				//SendUart(1);
				PwmCurve(100, 66);
			}
		}
		else if ((Control.ControlCommand & MaskRemoteControlRunL) == RemoteControlRunM)
		{	if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(200, 96); //PwmOut(2, 32 * 3);
			}
		}
		else if ((Control.ControlCommand & MaskRemoteControlRunL) == RemoteControlRunH)
		{	if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(300, 255); //PwmOut(3, 255);
			}
		}
		else if ((Control.ControlCommand & MaskRemoteControlBack) == RemoteControlBack)
		{
			if (0 == OutF) {
				Control.ControlCommand = -1;
				OutF = 0; OutB = 1;
				PwmCurve(200, 78);//PwmOut(2, 26 * 3);
			}
		}
		else if (Control.ControlCommand == ManualControlRunM)
		{
			if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(300, 96);//PwmOut(3, 32 * 3);
			}
		}
		else if (Control.ControlCommand == ManualControlRunH)
		{
			if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(500, 255); //PwmOut(5, 255);
			}
		}
		else if (Control.ControlCommand == ManualControlBack)
		{
			if (0 == OutF) {
				Control.ControlCommand = -1;
				OutF = 0; OutB = 1;
				PwmCurve(300, 78);//PwmOut(3, 26 * 3);
			}
		}
		if (Control.Turning)
		{
			Control.Turning++;
			if (Control.Turning > CONTROL_TRUNING_IDLE)
			{
				EA = 0;
				bLeft  = 0;
				bRight = 0;
				EA = 1;
				Control.Turning = 0;
			}
		}
		if (Control.Runing)
		{
			Control.Runing++;
			if (Control.Runing > CONTROL_RUNING_IDLE)
			{
				st = 1;
				EA = 0;
				STRPWM.PwmTime = 0;
				EA = 1;
				Control.Runing = 0;
			}
		}
	}
	_EE
}
int TaskControl2(void)//stops slowly
{
	_SS
	while (1)
	{
		if (st)
		{
			st = 0;
			PWMSETATOMIC(0);
			if (1 == OutB)
			{
				WaitX(800);
			}
			else
			{
				WaitX(1000);
			}
			if (st)continue;
			if (Control.Runing == 0) {OutF = 0; OutB = 0;}
		} WaitX(1);
	}
	_EE
}
#define FIFONUM 4
int TaskRf(void)
{	static u8 RfFifo[FIFONUM];
	_SS
	InitLT8900();
	spiWriteReg(7, 0x00, 0x30);
	delayMs(3);
	spiWriteReg(52, 0x00, 0x80);
	spiWriteReg(7, 0x00, 0xB0);
	delayMs(5);
	for (i = 0; i < 10; i++)//Calibration address code
	{
		spiWriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
		spiWriteReg(52, 0x80, 0x00);

		spiWriteReg(50, 5, *(pIdRam + 6));
		spiWriteReg(50, *(pIdRam + 0), *(pIdRam + 1));
		spiWriteReg(50, *(pIdRam + 2), *(pIdRam + 3));
		spiWriteReg(7, 0x01, 0x30);
		WaitX(20);
	}

	spiWriteReg(36, *(pIdRam + 0), *(pIdRam + 1));//Modify the address code
	spiWriteReg(38, *(pIdRam + 2), *(pIdRam + 3));
	spiWriteReg(39, 0xbd, *(pIdRam + 6));
	spiWriteReg(52, 0x00, 0x80);
	spiWriteReg(7, 0x00, 0xB0);
	delayMs(5);
	while (1)
	{
		static unsigned char lasti;
		u8 Data;
		u8 SerialNumber;
		WaitX(2);
		spiReadreg(48);
		if (0x40 == (RegL & 0x40))//Determine whether received data
		{
			spiReadreg(48);
			if (0x00 == (RegH & 0x80))//Verify the CRC
			{
				Control.NoReceiving = 0;
				spiReadreg(50); RfFifo[0] = RegH; RfFifo[1] = RegL;//Read the data
				if ((FIFONUM - 1) == RfFifo[0])//Check the FIFO size
				{
					spiReadreg(50); RfFifo[2]  = RegH; RfFifo[3] = RegL;//Read the data
					SerialNumber = (u8)((u8)RfFifo[1] - (u8)lasti) > 10 ? (lasti - RfFifo[1]) : (RfFifo[1] - lasti);//Check the serial number
					if (1 == SerialNumber)
					{
						Data = RfFifo[3];
					}
					else
					{
						Data = RfFifo[2];
						if (Data != 0xff)
						{
							Control.ControlCommand = Data ;
							if (((Data  & 0x84) == 0x04) || ((Data  & (0x80 | RemoteControlSpeed)) == RemoteControlSpeed))//If there is a speed data
							{
								if ((Data & 0x03) < 3)
									Control.Speed = (Data & 0x03);
							}
							WaitX(2);
							Data = RfFifo[3];
						}
					}
					lasti = RfFifo[1];
					if (Data != 0xff)
					{
						Control.ControlCommand = Data ;
						if (((Data  & 0x84) == 0x04) || ((Data  & (0x80 | RemoteControlSpeed)) == RemoteControlSpeed))//If there is a speed data
						{
							if ((Data & 0x03) < 3)
								Control.Speed = (Data & 0x03);
						}
					}
					SendUart(Control.ControlCommand);
					SendUart(Control.ControlCommand);
					SendUart(Control.ControlCommand);
					
					
				}
			}
			spiWriteReg(7, 0x00, 0x30);
			spiWriteReg(52, 0x80, 0x80);
			spiWriteReg(7, 0x00, 0xB0);
		}
		else if (Control.NoReceiving <= CONTROL_TRUNING_IDLE)
		{
			Control.NoReceiving++;
		}
	}
	_EE
}
#include "pwm.h"

void main(void)
{
	u8 it = 0;
	P3M0 = 0x6A;
	OutR = 0;
	OutL = 0;
	OutF = 0;
	OutB = 0;

	PCA_config(); //PWM
	Timer0Init(); //定时器（1ms 时间基数）
	//CCAP0H = 0xff;
	// for (i = 0; i < 200; i++)
	// {
	// 	PWMSETATOMIC(it++);
	// 	delayMs(20);
	// }
	PWMSETATOMIC(0);

	P_SW1 |= 0x80; //P_SW1 0x80 USART RE RX P1.6 TX P1.7
	UartInit();
	for (i = 0; i < 7; i++)SendUart(*pIdRam++);
	pIdRam = ID_ADDR_RAM;
	//Timer2Init();

	INMF = 1;
	INMB = 1;
	for (;;)
	{
		RunTaskA(TaskControl, 0);
		RunTaskA(TaskControl2, 1);
		RunTaskA(TaskRf, 2);
	}
}
