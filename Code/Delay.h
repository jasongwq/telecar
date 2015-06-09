#ifndef _DELAY_H
#define _DELAY_H

//bit fTimer1ms;
//void delayMs(unsigned int timerCnt)
//{
//    do
//    {
//        fTimer1ms = 0;
//        while (fTimer1ms == 0);
//    }
//    while (timerCnt--);
//}
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
    TL0 = 0x40;     //设置定时初值
    TH0 = 0xA2;     //设置定时初值
    TF0 = 0;        //清除TF0标志
    TR0 = 1;        //定时器0开始计时
    ET0 = 1;                        //使能定时器0中断
    EA = 1;
}
volatile int t = 0;
bit t1 = 0;



#endif
