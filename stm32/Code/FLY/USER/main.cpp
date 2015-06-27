/***SYS***/
#include "sys.h"
#include "gpio.h"
#include <string.h>
#include "usr_usart.h"
#include "minos_delay.h"
#include "sys_os.h"
#include "gpio.h"
#include "stm32_config.h"
#include "G32_timer.h"
/***C***/
#include "string.h"
#include "stdlib.h"
/***Other***/
//#include "aes.h"
//#include "adc.h"

#include "task_led.h"

void SYS_INIT(void)
{
    /***延时初始化***/
    delay_init();
    /***中断初始化***/
    NVIC_Configuration();
    uart_init (115200);
    uart2_init(115200);
    //    uart3_init(115200);
    Sys_Printf(USART1, (char *)"USART1 okhghg");
    Sys_Printf(USART2, (char *)"USART2 okrth5");
    //    Sys_Printf(USART3, (char *)"USART3 okewtr");
    delay_ms(100);
}

#include "delay.h"
#include "lt8910.h"
void InitLT8900(void)
{
    u8 EepromTmp[3 * 34] = {0x00, 0x01, 0x02, 0x04, 0x05, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x16, 0x17, 0x18, 0x19, 0x1a,
                            0x1b, 0x1c, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x32,
                            0x6f, 0xe0, 0x56, 0x81, 0x66, 0x17, 0x9c, 0xc9, 0x66, 0x37, 0x00, 0x00, 0x6c, 0x90, 0x48, 0x00, 0x7f,
                            0xfd, 0x00, 0x08, 0x00, 0x00, 0x48, 0xbd, 0x00, 0xff, 0x80, 0x05, 0x00, 0x67, 0x16, 0x59, 0x19, 0xe0,
                            0x13, 0x00, 0x18, 0x00, 0x48, 0x20, 0x3f, 0xc7, 0x20, 0x00, 0x03, 0x00, 0x03, 0x80, 0x03, 0x80, 0x03,
                            0x80, 0x03, 0x80, 0x44, 0x02, 0xb0, 0x00, 0xfd, 0xb0, 0x00, 0x0f, 0x01, 0x00, 0x04, 0x80, 0x00, 0x00
                           };
    RESET_N_Init;
    LT8910_Init;
    RESET_N = 0;
    delay_ms(100);
    RESET_N = 1;
    delay_ms(200);
    //SCLK = 0;
    SPI0_Init();

    for (u8 i = 0; i < 34; i++)spiWriteReg(EepromTmp[i], EepromTmp[i + i + 34], EepromTmp[i + i + 34 + 1]);
}
void RfSend(u8 Data)
{
    static unsigned char i = 0;
    static unsigned char lastdata = 0;

    spiWriteReg(7, 0x00, 0x30);             // 2402 + 48 = 2.45GHz
    spiWriteReg(52, 0x80, 0x00);            // 清空发送缓存区
    // 发送1个字节
    spiWriteReg(50, 3, i++ );
    spiWriteReg(50, lastdata, Data );
    spiWriteReg(7, 0x01, 0x30);             // 允许发射使能
    lastdata = Data;
}
void debuglt8910(void)
{
    spiReadreg(0);  Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(1 ); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(2 ); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(4 ); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(5 ); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(7 ); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(8 ); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(9 ); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(10); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(11); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(12); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(13); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(22); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(23); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(24); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(25); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(26); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(27); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(28); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(32); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(33); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(34); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(35); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(36); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(37); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(38); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(39); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(40); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(41); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(42); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(43); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
    spiReadreg(50); Sys_Printf(USART2, (char *)"\r\n %d,%d", RegH, RegL);
}
int TaskS(void)
{
    _SS
    InitLT8900();
    debuglt8910();
    spiWriteReg(7, 0x00, 0x30);
    delay_ms(3);
    spiWriteReg(52, 0x00, 0x80);            // 清接收缓存区
    spiWriteReg(7, 0x00, 0xB0);             // 允许接收使能
    delay_ms(5);
    while (1)
    {
        static int i = 0;
        WaitX(10);
        RfSend(i++);
    }
    _EE
}

int main(void)
{
    SYS_INIT();
    /***总循环***/
    while (1)
    {
        RunTaskA(TaskS, 0);
        //RunTaskA(TaskRf,1);
        RunTaskA(TaskLed, 2);
    }
}
