
#ifndef	__EEPROM_H
#define	__EEPROM_H

#include "STC15W401.h"
//#define MAIN_Fosc		24000000L	//������ʱ��


//	ѡ��MCU�ͺ�
#define	MCU_Type	STC15F_L2K08S2	//STC15F_L2K08S2, STC15F_L2K16S2, STC15F_L2K24S2, STC15F_L2K32S2, STC15F_L2K40S2, STC15F_L2K48S2, STC15F_L2K56S2, STC15F_L2K60S2, IAP15F_L2K61S2


/************************** ISP/IAP *****************************
 IAPϵ�� ������Ӧ�ó����޸�Ӧ�ó���

STC15F/L2KxxS2	�������䣬512�ֽ�/��������0x0000��ʼ��

     �ͺ�        ��С   ������  ��ʼ��ַ  ������ַ   MOVC��ƫ�Ƶ�ַ
STC15F/L2K08S2   53K   106����  0x0000  ~  0xD3FF        0x2000
STC15F/L2K16S2   45K    90����  0x0000  ~  0xB3FF        0x4000
STC15F/L2K24S2   37K    74����  0x0000  ~  0x93FF        0x6000
STC15F/L2K32S2   29K    58����  0x0000  ~  0x73FF        0x8000
STC15F/L2K40S2   21K    42����  0x0000  ~  0x53FF        0xA000
STC15F/L2K48S2   13K    26����  0x0000  ~  0x33FF        0xC000
STC15F/L2K56S2   5K     10����  0x0000  ~  0x13FF        0xE000
STC15F/L2K60S2   1K      2����  0x0000  ~  0x03FF        0xF000

STC15F/L2K61S2   ��EPROM, ����122������FLASH�����Բ�д ��ַ 0x0000~0xF3ff.

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
	ISP_CONTR = 0;			//��ֹISP/IAP����
	ISP_CMD   = 0;			//ȥ��ISP/IAP����
	ISP_TRIG  = 0;			//��ֹISP/IAP�����󴥷�
	ISP_ADDRH = 0xff;		//��0��ַ���ֽ�
	ISP_ADDRL = 0xff;		//��0��ַ���ֽڣ�ָ���EEPROM������ֹ�����
}

//========================================================================
// ����: void EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number)
// ����: ��ָ��EEPROM�׵�ַ����n���ֽڷ�ָ���Ļ���.
// ����: EE_address:  ����EEPROM���׵�ַ.
//       DataAddress: �������ݷŻ�����׵�ַ.
//       number:      �������ֽڳ���.
// ����: non.
// �汾: V1.0, 2012-10-22
//========================================================================

void EEPROM_read_n(u16 EE_address,u8 *DataAddress,u16 number)
{
	EA = 0;		//��ֹ�ж�
	ISP_CONTR = (ISP_EN + ISP_WAIT_FREQUENCY);	//���õȴ�ʱ�䣬����ISP/IAP��������һ�ξ͹�
	ISP_READ();									//���ֽڶ���������ı�ʱ����������������
	do
	{
		ISP_ADDRH = EE_address / 256;		//�͵�ַ���ֽڣ���ַ��Ҫ�ı�ʱ���������͵�ַ��
		ISP_ADDRL = EE_address % 256;		//�͵�ַ���ֽ�
		ISP_TRIG();							//����5AH������A5H��ISP/IAP�����Ĵ�����ÿ�ζ���Ҫ���
											//����A5H��ISP/IAP������������������
											//CPU�ȴ�IAP��ɺ󣬲Ż����ִ�г���
		_nop_();
		*DataAddress = ISP_DATA;			//��������������
		EE_address++;
		DataAddress++;
	}while(--number);

	DisableEEPROM();
	EA = 1;		//���������ж�
}


/******************** ������������ *****************/
//========================================================================
// ����: void EEPROM_SectorErase(u16 EE_address)
// ����: ��ָ����ַ��EEPROM��������.
// ����: EE_address:  Ҫ����������EEPROM�ĵ�ַ.
// ����: non.
// �汾: V1.0, 2013-5-10
//========================================================================
void EEPROM_SectorErase(u16 EE_address)
{
	EA = 0;		//��ֹ�ж�
											//ֻ������������û���ֽڲ�����512�ֽ�/������
											//����������һ���ֽڵ�ַ����������ַ��
	ISP_ADDRH = EE_address / 256;			//��������ַ���ֽڣ���ַ��Ҫ�ı�ʱ���������͵�ַ��
	ISP_ADDRL = EE_address % 256;			//��������ַ���ֽ�
	ISP_CONTR = (ISP_EN + ISP_WAIT_FREQUENCY);	//���õȴ�ʱ�䣬����ISP/IAP��������һ�ξ͹�
	ISP_ERASE();							//������������������ı�ʱ����������������
	ISP_TRIG();
	_nop_();
	DisableEEPROM();
	EA = 1;		//���������ж�
}

//========================================================================
// ����: void EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number)
// ����: �ѻ����n���ֽ�д��ָ���׵�ַ��EEPROM.
// ����: EE_address:  д��EEPROM���׵�ַ.
//       DataAddress: д��Դ���ݵĻ�����׵�ַ.
//       number:      д����ֽڳ���.
// ����: non.
// �汾: V1.0, 2012-10-22
//========================================================================
void EEPROM_write_n(u16 EE_address,u8 *DataAddress,u16 number)
{
	EA = 0;		//��ֹ�ж�

	ISP_CONTR = (ISP_EN + ISP_WAIT_FREQUENCY);	//���õȴ�ʱ�䣬����ISP/IAP��������һ�ξ͹�
	ISP_WRITE();							//���ֽ�д��������ı�ʱ����������������
	do
	{
		ISP_ADDRH = EE_address / 256;		//�͵�ַ���ֽڣ���ַ��Ҫ�ı�ʱ���������͵�ַ��
		ISP_ADDRL = EE_address % 256;		//�͵�ַ���ֽ�
		ISP_DATA  = *DataAddress;			//�����ݵ�ISP_DATA��ֻ�����ݸı�ʱ����������
		ISP_TRIG();
		_nop_();
		EE_address++;
		DataAddress++;
	}while(--number);

	DisableEEPROM();
	EA = 1;		//���������ж�
}

#endif