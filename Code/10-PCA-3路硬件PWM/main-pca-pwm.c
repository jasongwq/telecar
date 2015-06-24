
#include	"config.h"
#include	"PCA.h"
//#include	"delay.h"

/*************	功能说明	**************

输出3路变化的PWM信号。类似"呼吸灯"的驱动.

PWM0  为8位PWM.
PWM1  为7位PWM.
PWM2  为6位PWM.


******************************************/


/*************	本地变量声明	**************/

u8	pwm0;
bit	B_PWM0_Dir;	//方向, 0为+, 1为-.


void	PCA_config(void)
{
	PCA_InitTypeDef		PCA_InitStructure;

	PCA_InitStructure.PCA_Clock    = PCA_Clock_1T;		//PCA_Clock_1T, PCA_Clock_2T, PCA_Clock_4T, PCA_Clock_6T, PCA_Clock_8T, PCA_Clock_12T, PCA_Clock_Timer0_OF, PCA_Clock_ECI
	PCA_InitStructure.PCA_IoUse    = PCA_P12_P11_P10_P37;	//PCA_P12_P11_P10_P37, PCA_P34_P35_P36_P37, PCA_P24_P25_P26_P27
	PCA_InitStructure.PCA_Interrupt_Mode = DISABLE;		//ENABLE, DISABLE
	PCA_InitStructure.PCA_Polity   = PolityLow;			//优先级设置	PolityHigh,PolityLow
	PCA_InitStructure.PCA_RUN      = DISABLE;			//ENABLE, DISABLE
	PCA_Init(PCA_Counter,&PCA_InitStructure);

	PCA_InitStructure.PCA_Mode     = PCA_Mode_PWM;		//PCA_Mode_PWM, PCA_Mode_Capture, PCA_Mode_SoftTimer, PCA_Mode_HighPulseOutput
	PCA_InitStructure.PCA_PWM_Wide = PCA_PWM_8bit;		//PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit
	PCA_InitStructure.PCA_Interrupt_Mode = DISABLE;		//PCA_Rise_Active, PCA_Fall_Active, ENABLE, DISABLE
	PCA_InitStructure.PCA_Value    = 128 << 8;			//对于PWM,高8位为PWM占空比
	PCA_Init(PCA0,&PCA_InitStructure);

	CR = 1;
}


/******************** task A **************************/
void main(void)
{
	
	PCA_config();
	pwm0 = 128;

	B_PWM0_Dir = 0;

	UpdatePwm(PCA0,pwm0);

//	EA = 1;
	
	while (1)
	{
//		delay_ms(20);

		if(B_PWM0_Dir)
		{
				if(--pwm0 <= 16)	B_PWM0_Dir = 0;	//8位PWM
		}
		else	if(++pwm0 >= 240)	B_PWM0_Dir = 1;	//8位PWM
		UpdatePwm(PCA0,pwm0);
	}
}



