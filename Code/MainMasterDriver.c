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
    T2L = BAUD;         //�趨��ʱ��ֵ
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
    spiWriteReg(52, 0x00, 0x80);            // ����ջ�����
    spiWriteReg(7, 0x00, 0xB0);             // �������ʹ��
    delayMs(5);

    smState = SENDDATA;
    while (1)
    {
        if (smState == SENDDATA)
        {
            spi2WriteReg(7, 0x00, 0x00);             // 2402 + 48 = 2.45GHz
            spi2WriteReg(52, 0x80, 0x00);            // ��շ��ͻ�����

            // ����5���ֽ�
            spi2WriteReg(50, 5, 0x55);
            spi2WriteReg(50, 2, 3);
            spi2WriteReg(50, 4, 5);

            spi2WriteReg(7, 0x01, 0x30);             // ������ʹ��

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
                spiWriteReg(52, 0x00, 0x80);            // ����ջ�����
                spiWriteReg(7, 0x00, 0xB0);             // �������ʹ��
                delayMs(5);
            }
        }
    }
}
