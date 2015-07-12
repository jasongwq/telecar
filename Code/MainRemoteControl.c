#include "user.h"
#include "STC15W401.h"
#include "spi.h"
#include <intrins.h>
#include "delay.h"
#include "eeprom.h"
//#include "usart.h"
#include "lt8910.h"

bit fTimer10ms     = 0;
bit KeyRealse      = 0;
char Speed         = 0;
char LastKeyNumber = 0;
u16 KeyCount       = 0;
u16 SleepCount     = 0;
u8 AddressFrequency[5];
volatile u8 Key;
u8 SleepSave;

sbit    KeyT = P3 ^ 0;           //output
sbit    KeyR = P3 ^ 1;           //output
sbit    KeyL = P3 ^ 2;           //output
sbit    KeyD = P3 ^ 3;           //input
sbit    KeyS = P3 ^ 4;           //output
sbit    KeyB = P1 ^ 0;           //input
sbit    KeyF = P1 ^ 1;           //output

sbit    LEDF = P3 ^ 7;           //output
sbit    LEDH = P5 ^ 5;           //output
sbit    LEDM = P5 ^ 4;           //output
sbit    LEDL = P1 ^ 7;           //output

// #define Left                 0x10
// #define Right                0x18
// #define Skid                 0x80
// #define RemoteControlSpeed   0x20
// #define RemoteControlRun     0x04
// #define RemoteControlRunH    0x06
// #define RemoteControlRunM    0x05
// #define RemoteControlRunL    0x04
// #define RemoteControlBack    0x81
// #define ProofreadingFrequency 0x82


#define SETLEDH 0x40
#define SETLEDM 0x20
#define SETLEDL 0x10
#define SETLEDSPEEDH ((0x70)^(SETLEDH|SETLEDM|SETLEDL))
#define SETLEDSPEEDM ((0x70)^(SETLEDM|SETLEDL))
#define SETLEDSPEEDL ((0x70)^(SETLEDL))
#define SETLEDOFF ((0x70)^(0x00))


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

void LED(u8 Stata)
{
    LEDH = (Stata >> 6);
    LEDM = (Stata >> 5) & 0x01;
    LEDL = (Stata >> 4) & 0x01;
}
u8 KeyScan(void)
{
    u8 Keyt = 0;
    if (0      == KeyT)Keyt |= RemoteControlSpeed; //����
    if (0      == KeyR)Keyt |= Right; //��
    else if (0 == KeyL)Keyt |= Left; //��
    if (0      == KeyF)Keyt |= RemoteControlRun | ((Speed > 0x02) ? 0x01 : Speed); //ǰ
    else if (0 == KeyB)Keyt |= RemoteControlBack;  //��
    if (0      == KeyD)return 0x82; //��Ƶ
    if (0      == KeyS)return Skid;  //ɲ��
    return Keyt;
}

void SetLT9010Address(void)
{
    spiWriteReg(39, 0xbd, AddressFrequency[0]);
    spiWriteReg(36, AddressFrequency[1], AddressFrequency[2]);
    spiWriteReg(38, AddressFrequency[3], AddressFrequency[4]);
}
void FunProofreadingFrequency(void)//��Ƶ
{
    KeyCount++; //SendUart(KeyCount);
    if (KeyCount > 100)
    {
        spiWriteReg(36, 0x03, 0x80);
        spiWriteReg(38, 0x5a, 0x5a);
        spiWriteReg(39, 0x03, 0x80);

        spiWriteReg(7, 0x00, 0x30);
        delayMs(3);
        spiWriteReg(52, 0x00, 0x80);            // ����ջ�����
        spiWriteReg(7, 0x00, 0xB0);             // �������ʹ��
        delayMs(50);
        LEDF = 0;
        for (;;)
        {
            spiReadreg(48);
            if (0x40 == (RegL & 0x40))
            {
                spiReadreg(48);
                if (0x00 == (RegH & 0x80))
                {
                    spiReadreg(50);
                    if (0x05 == RegH)
                    {
                        AddressFrequency[0] = RegL;
                        spiReadreg(50); AddressFrequency[1] = RegH; AddressFrequency[2] = RegL;
                        spiReadreg(50); AddressFrequency[3] = RegH; AddressFrequency[4] = RegL;
                        SetLT9010Address();
                        EEPROM_SectorErase(0x0000);
                        EEPROM_write_n(0x0000, AddressFrequency, 5);
                        LEDF = 1;
                        break;
                    }
                }
                spiWriteReg(52, 0x80, 0x80);            // ����ջ�����
                spiWriteReg(7, 0x00, 0xB0);             // �������ʹ��
            }
            delayMs(2);
        }
    }
}
void RfSend(u8 Data)
{
    static unsigned char i = 0;
    static unsigned char lastdata = 0;
//SendUart(Data);
    spiWriteReg(7, 0x00, 0x30);             // 2402 + 48 = 2.45GHz
    spiWriteReg(52, 0x80, 0x00);            // ��շ��ͻ�����
    // ����1���ֽ�
    spiWriteReg(50, 3, i++ );
    spiWriteReg(50, lastdata, Data );
    spiWriteReg(7, 0x01, 0x30);             // ������ʹ��
    lastdata = Data;
}
#define SLEEPCOUNT 700

void Timer0InitS(void)		//1����@12.000MHz
{
	AUXR |= 0x80;		//��ʱ��ʱ��1Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0x20;		//���ö�ʱ��ֵ
	TH0 = 0xD1;		//���ö�ʱ��ֵ
    TF0 = 0;        //���TF0��־
    TR0 = 1;        //��ʱ��0��ʼ��ʱ
    ET0 = 1;        //ʹ�ܶ�ʱ��0�ж�
    EA = 1;
}
#define CCP_S0 0x10
#define CCP_S1 0x20

void main(void)
{
    //UartInit();
    //SendUart(0x11);
    {
        {
            IT0 = 1;
            EX0 = 1;
        }//INT0
        {
            IT1 = 1;
            EX1 = 1;
        }//INT1
        {
            INT_CLKO |= 0x40;
        }//INT4
        {
            IE2 |= 0x04;
        }//T2
        {
            ACC = P_SW1;
            ACC &= ~(CCP_S0 | CCP_S1);
            P_SW1 = ACC;//S0 S0

            CCON = 0;
            CL = 0;
            CH = 0;
            CMOD = 0x08;

            CCAP0L = 0;
            CCAP0H = 0;
            CCAPM0 = 0x11;

            CCAP1L = 0;
            CCAP1H = 0;
            CCAPM1 = 0x11;

            CR = 1;
        }
    }

    Timer0InitS();
    InitLT8900();
    EEPROM_read_n(0x0000, AddressFrequency, 5);
    SetLT9010Address();
    KeyT = 1;
    KeyR = 1;
    KeyL = 1;
    KeyD = 1;
    KeyS = 1;
    KeyB = 1;
    KeyF = 1;

    LEDL = 0;
    LEDH = 1;
    LEDM = 1;
    LEDF = 1;

    IT0 = 1;
    EX0 = 1;
    EA = 1;
    while (1)
    {
//        if ((SleepSave & 0x01) == 0x01)//ģ������
//        {
//            if (fTimer10ms == 1)
//            {
//                Key = KeyScan();
//                if (0 != Key)
//                {
//                    SleepSave = 0;
//                    LEDF = (SleepSave >> 7) & 0x01;
//                    LED(SleepSave);
//                }
//            }
//        }
//        else
        {
            Key = KeyScan();
            if (fTimer10ms == 1)
            {
                //unsigned char a=0x82;
                fTimer10ms = 0;
                if (0 != Key)
                {
                    KeyRealse = 1;
                    SleepCount = 0;
                    if (Key == ProofreadingFrequency)
                    {
                        FunProofreadingFrequency();
                    }
                    if (Key == RemoteControlSpeed)
                        RfSend((Speed + 1) > 2 ? (3 - Speed) : (Speed + 1));
                    else
                        RfSend(Key);
                }
                if (((0 == Key) || (LastKeyNumber != Key)) && KeyRealse)
                {
                    KeyRealse = 0;
                    KeyCount = 0;
                    //SendUart(LastKeyNumber);
                    if ((LastKeyNumber & (0x80 | RemoteControlSpeed)) == RemoteControlSpeed)
                    {
                        switch (++Speed)
                        {   //�͵�ƽ��
                        case 4: Speed = 0; LED(SETLEDSPEEDL); break;//L��
                        case 1: LED(SETLEDSPEEDM); break;//M��
                        case 2: LED(SETLEDSPEEDH); break;//H��
                        case 3: LED(SETLEDSPEEDM); break;//M��
                        default: Speed = 0; break;
                        }
                    }
                    else
                        RfSend(0xff);
                }
                else if (Key == 0) //Sleep
                {
                    if (SleepCount++ > SLEEPCOUNT)
                    {
                        SleepCount = 0;
                        SleepSave = (SleepSave << 1) | LEDF;
                        SleepSave = (SleepSave << 1) | LEDH;
                        SleepSave = (SleepSave << 1) | LEDM;
                        SleepSave = (SleepSave << 1) | LEDL;
                        SleepSave = (SleepSave << 4);
                        LEDF = 1;
                        LED(SETLEDOFF);
#if 0
                        SleepSave |= 0x01;
#else
                        PCON = 0x02;//����
                        _nop_();
                        _nop_();
                        _nop_();
                        _nop_();
                        LEDF = (SleepSave >> 7) & 0x01;
                        LED(SleepSave);
#endif
                    }
                }
                LastKeyNumber = Key;
            }
        }
    }
}
void exint0() interrupt 0
{
}
void exint1() interrupt 2
{
}
void exint4() interrupt 16
{
}
void pca_isr() interrupt 7
{
}

#define MS_COUNT 1//20
void tm2_isr() interrupt 12
{
}
void tm0_isr() interrupt 1 using 1
{
    static volatile int t = 0;
    EA = 0;
    fTimer1ms = 1;
    if (t++ > MS_COUNT)
    {
        t = 0;
        fTimer10ms = 1;
    }
    EA = 1;
}