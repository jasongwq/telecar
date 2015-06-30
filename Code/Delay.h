#ifndef _DELAY_H
#define _DELAY_H

bit fTimer1ms;
void delayMs(unsigned int timerCnt)
{
    do
    {
        fTimer1ms = 0;
        while (fTimer1ms == 0);
    }
    while (timerCnt--);
}
void Delay10us()        //@24.000MHz
{
    unsigned char i;

    _nop_();
    _nop_();
    i = 57;
    while (--i);
}

void Timer0Init(void)       //1毫秒@24.000MHz
{
    AUXR |= 0x80;       //定时器时钟1T模式
    TMOD &= 0xF0;       //设置定时器模式
    TL0 = 0x20;     //设置定时初值
    TH0 = 0xD1;     //设置定时初值
    TF0 = 0;        //清除TF0标志
    TR0 = 1;        //定时器0开始计时
    ET0 = 1;        //使能定时器0中断
    EA = 1;
}
//void Timer2Init(void)		//1毫秒@24.000MHz
//{
//	AUXR |= 0x04;		//定时器时钟1T模式
//	T2L = 0x40;		//设置定时初值
//	T2H = 0xA2;		//设置定时初值
//	IE2=0xAF;
//	AUXR |= 0x10;		//定时器2开始计时
//	
//}





#endif
