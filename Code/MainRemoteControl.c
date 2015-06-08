#include "spi.h"
#include <intrins.h>
#define MCU_FREQ                             24000000 // 设置晶振频率

#define IDLE        0
#define STANDBY     1
#define RECEIVE     2
#define SENDDATA    3
#define TESTPA      4
#define ACK         5
#define NO_CHANGE   255

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

void Delay10us(void);
void delayMs(unsigned int timerCnt);
void Timer1Init(void);

unsigned char smState, ackOk;
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
    TL0 = 0x40;     //设置定时初值
    TH0 = 0xA2;     //设置定时初值
    TF0 = 0;        //清除TF0标志
    TR0 = 1;        //定时器0开始计时
    ET0 = 1;                        //使能定时器0中断
    EA = 1;
}
void tm0_isr() interrupt 1 using 1
{
    EA = 0;
    fTimer1ms = 1;
    EA = 1;
}
unsigned char RegH, RegL;
unsigned char Reg2H, Reg2L;

unsigned char spiReadWrite(unsigned char Byte)
{
    unsigned char i;

    EA = 0;
    for (i = 0; i < 8; i++)
    {
        MOSI = (Byte & 0x80);
        SCLK = 1;
        Delay10us();
        Byte <<= 1;
        Byte |= MISO;
        SCLK = 0;
        Delay10us();
    }
    EA = 1;

    return (Byte);
}
unsigned char spi2ReadWrite(unsigned char Byte)
{
    unsigned char i;

    EA = 0;
    for (i = 0; i < 8; i++)
    {
        MOSI2 = (Byte & 0x80);
        SCLK2 = 1;
        Delay10us();
        Byte <<= 1;
        Byte |= MISO2;
        SCLK2 = 0;
        Delay10us();
    }
    EA = 1;

    return (Byte);
}
void spiReadreg(unsigned char reg)
{
    SS = 0;
    spiReadWrite(READ | reg);
    RegH = spiReadWrite(0x00);
    RegL = spiReadWrite(0x00);
    SS = 1;
}
void spiWriteReg(unsigned char reg, unsigned char byteH, unsigned char byteL)
{
    SS = 0;
    spiReadWrite(WRITE & reg);
    spiReadWrite(byteH);
    spiReadWrite(byteL);
    SS = 1;
}
void spi2Readreg(unsigned char reg)
{
    SS2 = 0;
    spi2ReadWrite(READ | reg);
    Reg2H = spi2ReadWrite(0x00);
    Reg2L = spi2ReadWrite(0x00);
    SS2 = 1;
}
void spi2WriteReg(unsigned char reg, unsigned char byteH, unsigned char byteL)
{
    SS2 = 0;
    spi2ReadWrite(WRITE & reg);
    spi2ReadWrite(byteH);
    spi2ReadWrite(byteL);
    SS2 = 1;
}
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
    spiWriteReg(13, 0x48, 0xbd);

    spiWriteReg(22, 0x00, 0xff);
    spiWriteReg(23, 0x80, 0x05);
    spiWriteReg(24, 0x00, 0x67);
    spiWriteReg(25, 0x16, 0x59);
    spiWriteReg(26, 0x19, 0xe0);
    spiWriteReg(27, 0x13, 0x00);
    spiWriteReg(28, 0x18, 0x00);

    spiWriteReg(32, 0x50, 0x00);
    spiWriteReg(33, 0x3f, 0xc7);
    spiWriteReg(34, 0x20, 0x00);
    spiWriteReg(35, 0x03, 0x00);
    spiWriteReg(36, 0x03, 0x80);
    spiWriteReg(37, 0x03, 0x80);
    spiWriteReg(38, 0x5a, 0x5a);
    spiWriteReg(39, 0x03, 0x80);
    spiWriteReg(40, 0x44, 0x01);
    spiWriteReg(41, 0xB0, 0x00);  //crc on scramble off ,1st byte packet length ,auto ack off
    spiWriteReg(42, 0xfd, 0xb0);  //
    spiWriteReg(43, 0x00, 0x0f);
    spiWriteReg(50, 0x00, 0x00);
    delayMs(200);

    spiWriteReg(7, 0x01, 0x00);
    delayMs(2);
    spiWriteReg(7, 0x00, 0x30);
}
//void Init2LT8900(void)
//{
//    RESET2_N = 0;
//    delayMs(100);
//    RESET2_N = 1;
//    delayMs(200);
//    SCLK2 = 0;

//    spi2WriteReg(0, 0x6f, 0xe0);
//    spi2WriteReg(1, 0x56, 0x81);
//    spi2WriteReg(2, 0x66, 0x17);
//    spi2WriteReg(4, 0x9c, 0xc9);
//    spi2WriteReg(5, 0x66, 0x37);
//    spi2WriteReg(7, 0x00, 0x00);
//    spi2WriteReg(8, 0x6c, 0x90);
//    spi2WriteReg(9, 0x48, 0x00);             // 5.5dBm
//    spi2WriteReg(10, 0x7f, 0xfd);
//    spi2WriteReg(11, 0x00, 0x08);
//    spi2WriteReg(12, 0x00, 0x00);
//    spi2WriteReg(13, 0x48, 0xbd);

//    spi2WriteReg(22, 0x00, 0xff);
//    spi2WriteReg(23, 0x80, 0x05);
//    spi2WriteReg(24, 0x00, 0x67);
//    spi2WriteReg(25, 0x16, 0x59);
//    spi2WriteReg(26, 0x19, 0xe0);
//    spi2WriteReg(27, 0x13, 0x00);
//    spi2WriteReg(28, 0x18, 0x00);

//    spi2WriteReg(32, 0x50, 0x00);
//    spi2WriteReg(33, 0x3f, 0xc7);
//    spi2WriteReg(34, 0x20, 0x00);
//    spi2WriteReg(35, 0x03, 0x00);
//    spi2WriteReg(36, 0x03, 0x80);
//    spi2WriteReg(37, 0x03, 0x80);
//    spi2WriteReg(38, 0x5a, 0x5a);
//    spi2WriteReg(39, 0x03, 0x80);
//    spi2WriteReg(40, 0x44, 0x01);
//    spi2WriteReg(41, 0xB0, 0x00);  //crc on scramble off ,1st byte packet length ,auto ack off
//    spi2WriteReg(42, 0xfd, 0xb0);  //
//    spi2WriteReg(43, 0x00, 0x0f);
//    spi2WriteReg(50, 0x00, 0x00);
//    delayMs(200);

//    spi2WriteReg(7, 0x01, 0x00);
//    delayMs(2);
//    spi2WriteReg(7, 0x00, 0x30);
//}
void main(void)
{

    UartInit();
    SendUart(0x11);
    Timer0Init();
    InitLT8900();
//    Init2LT8900();
    spiWriteReg(7, 0x00, 0x30);
    delayMs(3);
    spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
    spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
    delayMs(5);

    smState = SENDDATA;
    while (1)
    {
        if (smState == SENDDATA)
        {
            spi2WriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
            spi2WriteReg(52, 0x80, 0x00);            // 清空发送缓存区

            // 发送5个字节
            spi2WriteReg(50, 5, 0x55);
            spi2WriteReg(50, 2, 3);
            spi2WriteReg(50, 4, 5);

            spi2WriteReg(7, 0x01, 0x30);             // 允许发射使能

            do
            {
                spi2Readreg(48);
            }
            while (0 == (RegL >> 6 & 0x01));
            delayMs(300);
            smState = RECEIVE;
            SendUart(6);
        }
        else if (smState == RECEIVE)
        {
            spiReadreg(48);
            if (1 == (RegL >> 6 & 0x01))
            {
                spiReadreg(50);
                SendUart(RegL);
                SendUart(RegH);
                spiReadreg(50);
                SendUart(RegL);
                SendUart(RegH);
                spiReadreg(50);
                SendUart(RegL);
                SendUart(RegH);
                delayMs(200);
                spiWriteReg(7, 0x00, 0x30);
                delayMs(3);
                spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
                spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
                delayMs(5);
            }
        }
    }
}
