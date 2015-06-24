#include "data_transfer.h"
#include "sys.h"
#include "usr_usart.h"
#include "bak.h"
/*C/C++*/
#include "stdlib.h"
#include "string.h"
/*USAR*/
#include "minos_delay.h"
#include "aes.h"

/*Define*/
#define DATA_TRANSFER_USE_USART
#define SUM_CHECK_HOLD 0//1 OPEN 0 CLOSE
/*Static Fun*/
//static void Data_Send_Check(u16 check);
/*Variable*/
//struct DATA_TRANSFER_SWITCH Ex_ON_OFF, Send;

u16 AbsoluteOpticalEncoder_VAL = 0;//绝对式光电编码器
u8 RelayStata[8] = {0}; //继电器状态
_Uu32u8 TimeUnlock;//锁定时间
u8 TimeUnlockFlag=0;

u16 AbsoluteOpticalEncoder_Apart[8] =//继电器阈值
{
    30, 60, 90, 120,
    150, 180, 210, 360
};
union
{
    u8 u8[12];
    u32 u32[3];
} ChipUniqueID;
void Ex_Anl(u8 *data_buf)
{
    u8 LastUnlockKey[16];
    static u8 KeyUnlockFlag = 0;
    switch (*(data_buf + 2))
    {
    case 0X10://输入编码器角度
    {
        AbsoluteOpticalEncoder_VAL = *(data_buf + 4);
        for (int i = 0; i < 8; ++i)
        {
            if (AbsoluteOpticalEncoder_VAL < AbsoluteOpticalEncoder_Apart[i])
            {
                RelayStata[i] = 1;
                break;
            }
        }
        Sys_Printf(Printf_USART, "\r\nAbsoluteOpticalEncoder_VAL:%d", AbsoluteOpticalEncoder_VAL);
        Sys_Printf(Printf_USART, "\r\nRelayStata:%d", RelayStata);
        break;
    }
    case 0X11://修改单个继电器阈值
    {
        if (*(data_buf + 4) < 8)
            AbsoluteOpticalEncoder_Apart[*(data_buf + 4)] = *(data_buf + 5);
        Sys_Printf(Printf_USART, "\r\nAbsoluteOpticalEncoder_Apart:\r\n");
        for (int i = 0; i < 8; i++)Sys_Printf(Printf_USART, " %d", AbsoluteOpticalEncoder_Apart[i]);
        break;
    }
    case 0X12://设定TimeUnlock锁定时间
    {
        if (KeyUnlockFlag)
        {
            KeyUnlockFlag = 0;
            TimeUnlock.u8[0] = *(data_buf + 4);
					  TimeUnlock.u8[1] = *(data_buf + 5);
					  TimeUnlock.u8[2] = *(data_buf + 6);
					  TimeUnlock.u8[3] = *(data_buf + 7);		
        }
        Sys_Printf(Printf_USART, "\r\nTimeUnlock:%d", TimeUnlock);
        break;
    }
    case 0X13://得到解锁初始密钥
    {
        srand(SysTick_Clock());
        union
        {
            u8 u8[2];
            u16 u16;
        } TimeRand;
        TimeRand.u16 = rand();
        Sys_Printf(USART1, (char *)"\r\n%d", TimeRand.u16);
        Sys_Printf(USART1, (char *)"\r\n%d", TimeRand.u8[0]);
        Sys_Printf(USART1, (char *)"\r\n%d", TimeRand.u8[1]);


        unsigned char chainCipherBlock[16], dat[16] = {0};
        for (int i = 0; i < 12; ++i)dat[i] = ChipUniqueID.u8[i];
        for (int i = 0; i < 2; ++i)dat[i + 12] = TimeRand.u8[i];
        Sys_Printf(USART1, (char *)"\r\n"); for (int i = 0; i < 16; ++i)Sys_Printf(USART1, (char *)"%2X ", dat[i]);

        for (int i = 0; i < 32; i++) AES_Key_Table[i] = i; //做运算之前先要设置好密钥,这里只是设置密钥的DEMO

        memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));
        aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据.
        aesEncrypt(dat, chainCipherBlock);//AES加密,数组dat里面的新内容就是加密后的数据.
        for (int i = 0; i < 16; ++i)LastUnlockKey[i] = dat[i];

        Sys_Printf(USART1, (char *)"\r\n"); for (int i = 0; i < 16; ++i)Sys_Printf(USART1, (char *)"%2X ", dat[i]);

        memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock)); //这里要重新初始化清空
        aesDecInit();//在执行解密初始化之前可以为AES_Key_Table赋值有效的密码数据
        aesDecrypt(dat, chainCipherBlock);//AES解密，密文数据存放在dat里面，经解密就能得到之前的明文.

        Sys_Printf(USART1, (char *)"\r\n"); for (int i = 0; i < 16; ++i)Sys_Printf(USART1, (char *)"%2X ", dat[i]);
        break;
    }
    case 0X14://得到IC ID
    {
        ChipUniqueID.u32[2] = *(__IO u32 *)(0X1FFFF7E8); // 低字节
        ChipUniqueID.u32[1] = *(__IO u32 *)(0X1FFFF7EC); //
        ChipUniqueID.u32[0] = *(__IO u32 *)(0X1FFFF7F0); // 高字节
        Sys_Printf(USART1, (char *)"\r\nChipUniqueID: %8X %8X %8X", ChipUniqueID.u32[0], ChipUniqueID.u32[1], ChipUniqueID.u32[2]);
        Sys_Printf(USART1, (char *)"\r\nChipUniqueID: %2X%2X%2X%2X %2X%2X%2X%2X %2X%2X%2X%2X", ChipUniqueID.u8[0], ChipUniqueID.u8[1], ChipUniqueID.u8[2], ChipUniqueID.u8[3], ChipUniqueID.u8[4], ChipUniqueID.u8[5], ChipUniqueID.u8[6], ChipUniqueID.u8[7], ChipUniqueID.u8[8], ChipUniqueID.u8[9], ChipUniqueID.u8[10], ChipUniqueID.u8[11]);
        break;
    }
    case 0X15://输入解锁密钥 解锁控制权
    {
        unsigned char chainCipherBlock[16], dat[16] = {0};
        for (int i = 0; i < 16; ++i)dat[i] = *(data_buf + 4 + i);
        Sys_Printf(USART1, (char *)"\r\n"); for (int i = 0; i < 16; ++i)Sys_Printf(USART1, (char *)"%2X ", dat[i]);

        for (int i = 0; i < 32; i++) AES_Key_Table[i] = i; //做运算之前先要设置好密钥,这里只是设置密钥的DEMO

        memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock)); //这里要重新初始化清空
        aesDecInit();//在执行解密初始化之前可以为AES_Key_Table赋值有效的密码数据
        aesDecrypt(dat, chainCipherBlock);//AES解密，密文数据存放在dat里面，经解密就能得到之前的明文.

        Sys_Printf(USART1, (char *)"\r\n"); for (int i = 0; i < 16; ++i)Sys_Printf(USART1, (char *)"%2X ", dat[i]);
        Sys_Printf(USART1, (char *)"\r\n"); for (int i = 0; i < 16; ++i)Sys_Printf(USART1, (char *)"%2X ", LastUnlockKey[i]);
        {
            int i;
            for ( i = 0; i < 16; ++i)
                if (dat[i] != LastUnlockKey[i])
                    break;
            if (i == 16)
                KeyUnlockFlag = 1;
            else
                KeyUnlockFlag = 0;
        }
        break;
    }
    }
}

void Data_Receive_Anl(u8 *data_buf, u8 num)
{
    vs16 rc_value_temp;
    u8 sum = 0;
    //Sys_sPrintf(Printf_USART, data_buf, num);
    for (u8 i = 0; i < (num - 1); i++)
        sum += *(data_buf + i);
#if   SUM_CHECK_HOLD
    if (!(sum == *(data_buf + num - 1)))       return; //sum
#endif
    if (!(*(data_buf) == 0xAA && *(data_buf + 1) == 0xAF))     return; //
    Ex_Anl(data_buf);
}

void Data_Exchange(void)
{
#ifdef DATA_TRANSFER_USE_SPI_NRF
    Nrf_Check_Event();
    u8 sta = Nrf_Get_FIFOSta();
    if ((sta & (1 << 4)) == 0)
        return;
#endif
}

