#include "user.h"
#include "STC15W401.h"
#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "lt8910.h"
#include "usart.h"
#include "sys_os.h"
#include "pca.h"

#define CONTROL_RUNING_IDLE     (400)
#define CONTROL_TRUNING_IDLE    (80)
#define CONTROL_NORECIVING_IDLE (1000)

#define ID_ADDR_RAM 0xF1
#define PWMSETATOMIC(x) do{EA = 0;STRPWM.pwm0 = (x); CCAP0H = STRPWM.pwm0; EA = 1;}while(0);

#define BEGIN_Dutyfactor (36 )//起始脉宽 1.2s/8.2s*256
#define RUN_L_Dutyfactor (69 )//低速脉宽 2.2s
#define RUN_M_Dutyfactor (100)//中速脉宽 3.2s
#define RUN_H_Dutyfactor (255)//高速脉宽 高
#define BACK_Dutyfactor  (81 )//倒退脉宽 2.6s/8.2s*256

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
	volatile u16 Runing;
	volatile u16 Turning;
	volatile u16 NoReceiving;
	volatile u8 Speed;
	s16 RDirectionCount;
	s16 LDirectionCount;
} Control = { -1, 0, 0, 0, 0, 0, 0, 0};

u16 i1 = 0;

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
	                             0x80, 0x03, 0x80, 0x44, 0x02, 0xb0, 0x00, 0xfd, 0xb0, 0x00, 0x0f, 0x10, 0x00, 0x04, 0x80, 0x00, 0x00
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
	if (++i2 > 30)
	{
		i2 = 0;
		fTimer1ms = 1;
		if (bRight)
			if (Control.RDirectionCount > -1200) {Control.LDirectionCount = 0; Control.RDirectionCount--; OutR = 1;}
			else bRight = 0;
		else OutR = 0;
		if (bLeft)
			if (Control.LDirectionCount <  1200) {Control.RDirectionCount = 0; Control.LDirectionCount++; OutL = 1;}
			else bLeft = 0;
		else OutL = 0;
		UpdateTimers();
	}
	if (++i1 > 311)//PWM
	{
		i1 = 0;
		if (STRPWM.PwmTime > 1)
		{
			STRPWM.PwmTime--;
			STRPWM.pwm0 += STRPWM.PwmStep;
			if ((STRPWM.pwm0 > BEGIN_Dutyfactor) && (STRPWM.pwm0 < RUN_L_Dutyfactor))//加快低速时的加速速度
			{
				int i;
				for (i = 0; i < 2, STRPWM.PwmTime>1, STRPWM.PwmStep > 0; ++i) //为原来的三倍
				{
					STRPWM.PwmTime--;
					STRPWM.pwm0 += STRPWM.PwmStep;
				}
			}
			CCAP0H = STRPWM.pwm0;
		}
		else if (STRPWM.PwmTime == 1)
		{
			STRPWM.PwmTime--;
			STRPWM.pwm0 = STRPWM.TargetDutyfactor;
			CCAP0H = STRPWM.pwm0;
		}
	}

	EA = 1;
}

/**
 * [PwmCurve pwm曲线加速]
 * @param time             [调节所用时间]
 * @param TargetDutyfactor [目标占空比]
 */
void PwmCurve(u16 time, u8 TargetDutyfactor)
{
	s16 Dvalue;//差值
	Control.Runing = 1;
	STRPWM.TargetDutyfactor = TargetDutyfactor;//目标占空比
	EA      = 0;
	if (STRPWM.pwm0 < BEGIN_Dutyfactor)STRPWM.pwm0 = BEGIN_Dutyfactor;//起始脉宽
	Dvalue  = TargetDutyfactor - STRPWM.pwm0;
	STRPWM.PwmStep = (TargetDutyfactor - BEGIN_Dutyfactor) / (double)time;//单位时间步长
	STRPWM.PwmTime = Dvalue / STRPWM.PwmStep;//调节次数
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
			EA = 0; bLeft = 1; bRight = 0; EA = 1;
		}
		else if ((Control.ControlCommand & MaskRight) == Right)
		{
			Control.Turning = 1;
			Control.ControlCommand &= 0xe7;
			EA = 0; bLeft = 0; bRight = 1; EA = 1;
		}
		if (Control.ControlCommand == Skid)
		{	Control.ControlCommand = -1;
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
				PwmCurve(100, RUN_L_Dutyfactor);//一秒内1.2/8.2
			}
		}
		else if ((Control.ControlCommand & MaskRemoteControlRunL) == RemoteControlRunM)
		{	if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(200, RUN_M_Dutyfactor); //PwmOut(2, 32 * 3);
			}
		}
		else if ((Control.ControlCommand & MaskRemoteControlRunL) == RemoteControlRunH)
		{	if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(300, RUN_H_Dutyfactor); //PwmOut(3, 255);
			}
		}
		else if ((Control.ControlCommand & MaskRemoteControlBack) == RemoteControlBack)
		{
			if (0 == OutF) {
				Control.ControlCommand = -1;
				OutF = 0; OutB = 1;
				PwmCurve(200, BACK_Dutyfactor);//PwmOut(2, 26 * 3);
			}
		}
		else if (Control.ControlCommand == ManualControlRunM)
		{
			if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(300, RUN_M_Dutyfactor);//PwmOut(3, 32 * 3);
			}
		}
		else if (Control.ControlCommand == ManualControlRunH)
		{
			if (0 == OutB) {
				Control.ControlCommand &= 0xb8;
				OutB = 0; OutF = 1;
				PwmCurve(500, RUN_H_Dutyfactor); //PwmOut(5, 255);
			}
		}
		else if (Control.ControlCommand == ManualControlBack)
		{
			if (0 == OutF) {
				Control.ControlCommand = -1;
				OutF = 0; OutB = 1;
				PwmCurve(300, BACK_Dutyfactor);//PwmOut(3, 26 * 3);
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

char comp(int id)
{
	if (*(pIdRam + 5) != ((id >> 8) & 0xff))
		return 0;
	if (*(pIdRam + 6) != ((id >> 0) & 0xff))
		return 0;
	return 1;
}

int TaskRf(void)
{	static u8 RfFifo[FIFONUM];
	_SS
	InitLT8900();
	spiWriteReg(7, 0x00, 0x30);
	delayMs(3);
	spiWriteReg(52, 0x00, 0x80);
	spiWriteReg(7, 0x00, 0xB0);
	delayMs(5);
	for (i = 0; i < 50; i++)//Calibration address code
	{
		spiWriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
		spiWriteReg(52, 0x80, 0x00);

		spiWriteReg(50, 5, *(pIdRam + 6));
		spiWriteReg(50, *(pIdRam + 1), *(pIdRam + 2));
		spiWriteReg(50, *(pIdRam + 3), *(pIdRam + 5));
		spiWriteReg(7, 0x01, 0x30);
		WaitX(20);
	}
	while (1)
	{
		WaitX(100);
		if (comp(0x7727))
		{
			SendUart(0x1);
			break;
		}
		if (comp(0x6302))
		{
			SendUart(0x2);
			break;
		}
		if (comp(0x4E1D))
		{
			SendUart(0x3);
			break;
		}
		if (comp(0x62A4))
		{
			SendUart(0x4);
			break;
		}
		if (comp(0x4E1F))
		{
			SendUart(0x5);
			break;
		}
		if (comp(0x76C9))
		{
			SendUart(0x6);
			break;
		}
	}
	SendUart(0x10);

	spiWriteReg(36, *(pIdRam + 1), *(pIdRam + 2));//Modify the address code
	spiWriteReg(38, *(pIdRam + 3), *(pIdRam + 5));
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
				}
			}
			spiWriteReg(7, 0x00, 0x30);
			spiWriteReg(52, 0x80, 0x80);
			spiWriteReg(7, 0x00, 0xB0);
		}
		else if (Control.NoReceiving <= CONTROL_NORECIVING_IDLE)
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
	UartInit();
	PCA_config(); //PWM
	Timer0Init(); //定时器（1ms 时间基数）
	delayMs(500);
	PWMSETATOMIC(0);

	P_SW1 |= 0x80; //P_SW1 0x80 USART RE RX P1.6 TX P1.7
	pIdRam = ID_ADDR_RAM;

	INMF = 1;
	INMB = 1;
	for (;;)
	{
		RunTaskA(TaskControl, 0);
		RunTaskA(TaskControl2, 1);
		RunTaskA(TaskRf, 2);
	}
}
