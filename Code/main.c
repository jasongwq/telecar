#include "spi.h"
#include <intrins.h>
#define MCU_FREQ                             24000000 // ���þ���Ƶ��

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
    SCON = 0x5a;        //8λ����,�ɱ䲨����
    AUXR |= 0x01;       //����1ѡ��ʱ��2Ϊ�����ʷ�����
    AUXR |= 0x04;       //��ʱ��2ʱ��ΪFosc,��1T
    T2L = BAUD;     //�趨��ʱ��ֵ
    T2H = BAUD >> 8;    //�趨��ʱ��ֵ
    AUXR |= 0x10;       //������ʱ��2
}

/************************************************
�������ݵ�����
��ڲ���:
    dat : ׼�����͵�����
���ڲ���: ��
************************************************/
void SendUart(char dat)
{
    while (!TI);                                //�ȴ���һ�����ݷ������
    TI = 0;                                     //���������ɱ�־
    SBUF = dat;                                 //�������ε����ݷ���
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

void Timer0Init(void)       //1����@24.000MHz
{
    AUXR |= 0x80;       //��ʱ��ʱ��1Tģʽ
    TMOD &= 0xF0;       //���ö�ʱ��ģʽ
    TL0 = 0x40;     //���ö�ʱ��ֵ
    TH0 = 0xA2;     //���ö�ʱ��ֵ
    TF0 = 0;        //���TF0��־
    TR0 = 1;        //��ʱ��0��ʼ��ʱ
    ET0 = 1;                        //ʹ�ܶ�ʱ��0�ж�
    EA = 1;
}
void tm0_isr() interrupt 1 using 1
{
    EA = 0;
    fTimer1ms = 1;
    EA = 1;
}
unsigned char RegH, RegL;
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
#if 0
//void main(void)
//{
//    P1 = 0xFF;
//    P1M1 = 0x0A;
//    P1M0 = 0xF5;
//    /*
//        P1M1 = 0x03;
//        P1M0 = 0xFC;
//    */

//    Timer1Init();
//    InitLT8900();

//    smState = STANDBY;

//    while (1)
//    {
//        spiWriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
//        spiWriteReg(52, 0x80, 0x00);            // ��շ��ͻ�����

//        // ����5���ֽ�
//        spiWriteReg(50, 5, 0x55);
//        spiWriteReg(50, 2, 3);
//        spiWriteReg(50, 4, 5);

//        spiWriteReg(7, 0x01, 0x30);             // ������ʹ��

//        while (PKT == 0);
//        delayMs(300);
//    }
//}
#else
void main(void)
{

    UartInit();
    SendUart(0x11);
    Timer0Init();
    InitLT8900();
    spiWriteReg(7, 0x00, 0x30);
    delayMs(3);
    spiWriteReg(52, 0x00, 0x80);            // ����ջ�����
    spiWriteReg(7, 0x00, 0xB0);             // �������ʹ��
    delayMs(5);
	
spiReadreg(0);SendUart(RegL);SendUart(RegH);
spiReadreg(1 );SendUart(RegL);SendUart(RegH);
spiReadreg(2 );SendUart(RegL);SendUart(RegH);
spiReadreg(4 );SendUart(RegL);SendUart(RegH);
spiReadreg(5 );SendUart(RegL);SendUart(RegH);
spiReadreg(7 );SendUart(RegL);SendUart(RegH);
spiReadreg(8 );SendUart(RegL);SendUart(RegH);
spiReadreg(9 );SendUart(RegL);SendUart(RegH);
spiReadreg(10);SendUart(RegL);SendUart(RegH);
spiReadreg(11);SendUart(RegL);SendUart(RegH);
spiReadreg(12);SendUart(RegL);SendUart(RegH);
spiReadreg(13);SendUart(RegL);SendUart(RegH);
spiReadreg(22);SendUart(RegL);SendUart(RegH);
spiReadreg(23);SendUart(RegL);SendUart(RegH);
spiReadreg(24);SendUart(RegL);SendUart(RegH);
spiReadreg(25);SendUart(RegL);SendUart(RegH);
spiReadreg(26);SendUart(RegL);SendUart(RegH);
spiReadreg(27);SendUart(RegL);SendUart(RegH);
spiReadreg(28);SendUart(RegL);SendUart(RegH);
spiReadreg(32);SendUart(RegL);SendUart(RegH);
spiReadreg(33);SendUart(RegL);SendUart(RegH);
spiReadreg(34);SendUart(RegL);SendUart(RegH);
spiReadreg(35);SendUart(RegL);SendUart(RegH);
spiReadreg(36);SendUart(RegL);SendUart(RegH);
spiReadreg(37);SendUart(RegL);SendUart(RegH);
spiReadreg(38);SendUart(RegL);SendUart(RegH);
spiReadreg(39);SendUart(RegL);SendUart(RegH);
spiReadreg(40);SendUart(RegL);SendUart(RegH);
spiReadreg(41);SendUart(RegL);SendUart(RegH);
spiReadreg(42);SendUart(RegL);SendUart(RegH);
spiReadreg(43);SendUart(RegL);SendUart(RegH);
spiReadreg(50);SendUart(RegL);SendUart(RegH);
    while (1)
    {
        //        if (0 == PKT)
        //        {
        //            smState = STANDBY;
        //            spiReadreg(50);
        //            SendUart(RegL);
        //            SendUart(RegH);
        //            spiReadreg(50);
        //            SendUart(RegL);
        //            SendUart(RegH);
        //            spiReadreg(50);
        //            SendUart(RegL);
        //            SendUart(RegH);
        //            spiWriteReg(7, 0x00, 0x30);
        //            delayMs(3);
        //            spiWriteReg(52, 0x00, 0x80);            // ����ջ�����
        //            spiWriteReg(7, 0x00, 0xB0);             // �������ʹ��
        //            delayMs(5);
        //        }
        //                      spiReadreg(50);
        //            SendUart(RegL);
        //            SendUart(RegH);
    }
}
#endif
//-----------------------------------------------------------------------------
//unsigned char ScanKey(void)
//{
//    char a = 0;
//    unsigned char nextState = NO_CHANGE;

//    if (1 == a)
//        nextState = SENDDATA;
//    if (1 == a)
//        nextState = TESTPA;
//    if (1 == a)
//        nextState = SENDDATA;
//    if (1 == a)
//        nextState = TESTPA;
//    if (1 == a)
//        nextState = SENDDATA;
//    if (1 == a)
//        nextState = TESTPA;
//    if (1 == a)
//        nextState = SENDDATA;
//    if (1 == a)
//        nextState = TESTPA;

//    return (nextState);
//}

