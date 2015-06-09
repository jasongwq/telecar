#ifndef _USART_H
#define _USART_H
#define MCU_FREQ                             24000000 // 设置晶振频率

#define BAUD            (65536 - MCU_FREQ / 4 / 115200)

void UartInit(void)
{
    SCON = 0x5a;        //8位数据,可变波特率
    AUXR |= 0x01;       //串口1选择定时器2为波特率发生器
    AUXR |= 0x04;       //定时器2时钟为Fosc,即1T
    T2L = BAUD;         //设定定时初值
    T2H = BAUD >> 8;    //设定定时初值
    AUXR |= 0x10;       //启动定时器2
}
/************************************************
发送数据到串口
入口参数:
    dat : 准备发送的数据
出口参数: 无
************************************************/
void SendUart(char dat)
{
    while (!TI);                                //等待上一个数据发送完成
    TI = 0;                                     //清除发送完成标志
    SBUF = dat;                                 //触发本次的数据发送
}

#endif
