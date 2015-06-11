
#ifndef	__EEPROM_H
#define	__EEPROM_H

#include "STC15W401.h"
//#define MAIN_Fosc		24000000L	//定义主时钟


//	选择MCU型号
#define	MCU_Type	STC15F_L2K08S2	//STC15F_L2K08S2, STC15F_L2K16S2, STC15F_L2K24S2, STC15F_L2K32S2, STC15F_L2K40S2, STC15F_L2K48S2, STC15F_L2K56S2, STC15F_L2K60S2, IAP15F_L2K61S2


/************************** ISP/IAP *****************************
 IAP系列 可以在应用程序修改应用程序。

STC15F/L2KxxS2	扇区分配，512字节/扇区，从0x0000开始。

     型号        大小   扇区数  开始地址  结束地址   MOVC读偏移地址
STC15F/L2K08S2   53K   106扇区  0x0000  ~  0xD3FF        0x2000
STC15F/L2K16S2   45K    90扇区  0x0000  ~  0xB3FF        0x4000
STC15F/L2K24S2   37K    74扇区  0x0000  ~  0x93FF        0x6000
STC15F/L2K32S2   29K    58扇区  0x0000  ~  0x73FF        0x8000
STC15F/L2K40S2   21K    42扇区  0x0000  ~  0x53FF        0xA000
STC15F/L2K48S2   13K    26扇区  0x0000  ~  0x33FF        0xC000
STC15F/L2K56S2   5K     10扇区  0x0000  ~  0x13FF        0xE000
STC15F/L2K60S2   1K      2扇区  0x0000  ~  0x03FF        0xF000

STC15F/L2K61S2   无EPROM, 整个122扇区的FLASH都可以擦写 地址 0x0000~0xF3ff.

*/

//#if   (MCU_Type == STC15F_L2K08S2)
      #define   MOVC_ShiftAddress    0x2000
//#elif (MCU_Type == STC15F_L2K16S2)
//      #define   MOVC_ShiftAddress    0x4000
//#elif (MCU_Type == STC15F_L2K24S2
//      #define   MOVC_ShiftAddress    0x6000
//#elif (MCU_Type == STC15F_L2K32S2
//      #define   MOVC_ShiftAddress    0x8000
//#elif (MCU_Type == STC15F_L2K40S2
//      #define   MOVC_ShiftAddress    0xA000
//#elif (MCU_Type == STC15F_L2K48S2
//      #define   MOVC_ShiftAddress    0xC000
//#elif (MCU_Type == STC15F_L2K56S2
//      #define   MOVC_ShiftAddress    0xE000
//#elif (MCU_Type == STC15F_L2K60S2
//      #define   MOVC_ShiftAddress    0xF000
//#elif (MCU_Type == IAP15F_L2K61S2
//      #define   MOVC_ShiftAddress    0x0000
//#endif


void	DisableEEPROM(void);
void 	EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number);
void 	EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number);
void	EEPROM_SectorErase(u16 EE_address);

void	DisableEEPROM(void)
{
	ISP_CONTR = 0;			//禁止ISP/IAP操作
	ISP_CMD   = 0;			//去除ISP/IAP命令
	ISP_TRIG  = 0;			//防止ISP/IAP命令误触发
	ISP_ADDRH = 0xff;		//清0地址高字节
	ISP_ADDRL = 0xff;		//清0地址低字节，指向非EEPROM区，防止误操作
}

//========================================================================
// 函数: void EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number)
// 描述: 从指定EEPROM首地址读出n个字节放指定的缓冲.
// 参数: EE_address:  读出EEPROM的首地址.
//       DataAddress: 读出数据放缓冲的首地址.
//       number:      读出的字节长度.
// 返回: non.
// 版本: V1.0, 2012-10-22
//========================================================================

void EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number)
{
	EA = 0;		//禁止中断
	ISP_CONTR = (ISP_EN + ISP_WAIT_FREQUENCY);	//设置等待时间，允许ISP/IAP操作，送一次就够
	ISP_READ();									//送字节读命令，命令不需改变时，不需重新送命令
	do
	{
		ISP_ADDRH = EE_address / 256;		//送地址高字节（地址需要改变时才需重新送地址）
		ISP_ADDRL = EE_address % 256;		//送地址低字节
		ISP_TRIG();							//先送5AH，再送A5H到ISP/IAP触发寄存器，每次都需要如此
											//送完A5H后，ISP/IAP命令立即被触发启动
											//CPU等待IAP完成后，才会继续执行程序。
		_nop_();
		*DataAddress = ISP_DATA;			//读出的数据送往
		EE_address++;
		DataAddress++;
	}while(--number);

	DisableEEPROM();
	EA = 1;		//重新允许中断
}


/******************** 扇区擦除函数 *****************/
//========================================================================
// 函数: void EEPROM_SectorErase(u16 EE_address)
// 描述: 把指定地址的EEPROM扇区擦除.
// 参数: EE_address:  要擦除的扇区EEPROM的地址.
// 返回: non.
// 版本: V1.0, 2013-5-10
//========================================================================
void EEPROM_SectorErase(u16 EE_address)
{
	EA = 0;		//禁止中断
											//只有扇区擦除，没有字节擦除，512字节/扇区。
											//扇区中任意一个字节地址都是扇区地址。
	ISP_ADDRH = EE_address / 256;			//送扇区地址高字节（地址需要改变时才需重新送地址）
	ISP_ADDRL = EE_address % 256;			//送扇区地址低字节
	ISP_CONTR = (ISP_EN + ISP_WAIT_FREQUENCY);	//设置等待时间，允许ISP/IAP操作，送一次就够
	ISP_ERASE();							//送扇区擦除命令，命令不需改变时，不需重新送命令
	ISP_TRIG();
	_nop_();
	DisableEEPROM();
	EA = 1;		//重新允许中断
}

//========================================================================
// 函数: void EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number)
// 描述: 把缓冲的n个字节写入指定首地址的EEPROM.
// 参数: EE_address:  写入EEPROM的首地址.
//       DataAddress: 写入源数据的缓冲的首地址.
//       number:      写入的字节长度.
// 返回: non.
// 版本: V1.0, 2012-10-22
//========================================================================
void EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number)
{
	EA = 0;		//禁止中断

	ISP_CONTR = (ISP_EN + ISP_WAIT_FREQUENCY);	//设置等待时间，允许ISP/IAP操作，送一次就够
	ISP_WRITE();							//送字节写命令，命令不需改变时，不需重新送命令
	do
	{
		ISP_ADDRH = EE_address / 256;		//送地址高字节（地址需要改变时才需重新送地址）
		ISP_ADDRL = EE_address % 256;		//送地址低字节
		ISP_DATA  = *DataAddress;			//送数据到ISP_DATA，只有数据改变时才需重新送
		ISP_TRIG();
		_nop_();
		EE_address++;
		DataAddress++;
	}while(--number);

	DisableEEPROM();
	EA = 1;		//重新允许中断
}

#endif