#include "data_transfer.h"
#include "sys.h"
#include "usr_usart.h"
#include "bak.h"
/*C/C++*/
#include "stdlib.h"
/*USAR*/
#include "minos_delay.h"
#include "aes.h"

/*Define*/
#define DATA_TRANSFER_USE_USART
#define SUM_CHECK_HOLD 0//1 OPEN 0 CLOSE
/*Static Fun*/
static void Data_Send_Check(u16 check);
/*Variable*/
struct DATA_TRANSFER_SWITCH Ex_ON_OFF, Send;

u16 AbsoluteOpticalEncoder_VAL = 0;//绝对是光电编码器
u8 RelayStata;//继电器状态
u8 TimeUnlock;//时间锁

u16 AbsoluteOpticalEncoder_Apart[8] =
{
    30, 60, 90, 120,
    150, 180, 210, 360
};
union{
    u8 ChipUniqueID8[12];
    u32 ChipUniqueID32[3];
};
void Ex_Anl(u8 *data_buf)
{
    switch (*(data_buf + 2))
    {
    case 0X10:
    {
        AbsoluteOpticalEncoder_VAL = *(data_buf + 4);
        for (int i = 0; i < 8; ++i)
        {
            if (AbsoluteOpticalEncoder_VAL < AbsoluteOpticalEncoder_Apart[i])
            {
                RelayStata = i;
                break;
            }
        }
        Sys_Printf(Printf_USART, "\r\nAbsoluteOpticalEncoder_VAL:%d", AbsoluteOpticalEncoder_VAL);
        Sys_Printf(Printf_USART, "\r\nRelayStata:%d", RelayStata);
        break;
    }
    case 0X11:
    {
        if (*(data_buf + 4) < 8)
            AbsoluteOpticalEncoder_Apart[*(data_buf + 4)] = *(data_buf + 5);
        Sys_Printf(Printf_USART, "\r\nAbsoluteOpticalEncoder_Apart:\r\n");
        for (int i = 0; i < 8; i++)Sys_Printf(Printf_USART, " %d", AbsoluteOpticalEncoder_Apart[i]);
        break;
    }
    case 0X12:
    {
        TimeUnlock = *(data_buf + 4);
        Sys_Printf(Printf_USART, "\r\nTimeUnlock:%d", TimeUnlock);
        break;
    }
    case 0X13:
    {
        srand(SysTick_Clock());
        Sys_Printf(USART1, (char *)"\r\n%d", rand());

        unsigned char dat[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        unsigned char chainCipherBlock[16];
        unsigned char i;
        for (i = 0; i < 32; i++) AES_Key_Table[i] = i; //做运算之前先要设置好密钥，这里只是设置密钥的DEMO。

        memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));
        aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
        aesEncrypt(dat, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。
        //aesEncrypt(dat+16, chainCipherBlock);//AES源数据大于16字节时，把源数据的指针+16就好了

        Sys_Printf(USART1, (char *)"\r\n");
        for (int i = 0; i < 16; ++i)   Sys_Printf(USART1, (char *)"%X ", dat[i]);

        memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock)); //这里要重新初始化清空
        aesDecInit();//在执行解密初始化之前可以为AES_Key_Table赋值有效的密码数据
        aesDecrypt(dat, chainCipherBlock);//AES解密，密文数据存放在dat里面，经解密就能得到之前的明文。

        Sys_Printf(USART1, (char *)"\r\n");
        for (int i = 0; i < 16; ++i)   Sys_Printf(USART1, (char *)"%X ", dat[i]);
        break;
    }
    case 0X14:
    {
        ChipUniqueID32[2] = *(__IO u32 *)(0X1FFFF7E8); // 低字节
        ChipUniqueID32[1] = *(__IO u32 *)(0X1FFFF7EC); //
        ChipUniqueID32[0] = *(__IO u32 *)(0X1FFFF7F0); // 高字节
        Sys_Printf(USART1, (char *)"ChipUniqueID: %X %X %X", ChipUniqueID32[0], ChipUniqueID32[1], ChipUniqueID32[2]);
        Sys_Printf(USART1, (char *)"ChipUniqueID: %X %X %X %X %X %X %X %X %X %X %X %X", ChipUniqueID8[0], ChipUniqueID8[1], ChipUniqueID8[2],ChipUniqueID8[3], ChipUniqueID8[4], ChipUniqueID8[5],ChipUniqueID8[6], ChipUniqueID8[7], ChipUniqueID8[8],ChipUniqueID8[9], ChipUniqueID8[10], ChipUniqueID8[11]);
        break;
    }
    }
}

void Data_Receive_Anl(u8 *data_buf, u8 num)
{
    vs16 rc_value_temp;
    u8 sum = 0;
    Sys_sPrintf(Printf_USART, data_buf, num);
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

