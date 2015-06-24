#ifndef __ADC_CONFIG_
#define __ADC_CONFIG_

/****************************************/
/***ADC***/
/***************************************************|
| Channel   |    ADC1     |  ADC2      |  ADC3      |
|***********|*************|************|************|
| Channel0  | A0          |  A0        |  A0        |
| Channel1  | A1          |  A1        |  A1        |
| Channel2  | A2          |  A2        |  A2        |
| Channel3  | A3          |  A3        |  A3        |
| Channel4  | A4          |  A4        |  F6        |
| Channel5  | A5          |  A5        |  F7        |
| Channel6  | A6          |  A6        |  F8        |
| Channel7  | A7          |  A7        |  F9        |
| Channel8  | B0          |  B0        |  F10       |
| Channel9  | B1          |  B1        |            |
| Channel10 | C0          |  C0        |  C0        |
| Channel11 | C1          |  C1        |  C1        |
| Channel12 | C2          |  C2        |  C2        |
| Channel13 | C3          |  C3        |  C3        |
| Channel14 | C4          |  C4        |            |
| Channel15 | C5          |  C5        |            |
| Channel16 | 温度传感器  |           |            |
| Channel17 | 内部参考电压|           |            |
|***************************************************/
//是否使能DMA
/*
ex.
ADC1_Init();
ADC_SoftwareStartConvCmd(ADC1, ENABLE);
ADC_ConvertedValue[0]
*/
#define ADC1_DMA 1
#define ADC1_DMAIRQ_ENABLE 0
#define ADC1_Channel00ON ((1<<5)+0)
#define ADC1_Channel01ON ((1<<5)+0)
#define ADC1_Channel02ON ((0<<5)+0)
#define ADC1_Channel03ON ((0<<5)+0)
#define ADC1_Channel04ON ((0<<5)+0)
#define ADC1_Channel05ON ((0<<5)+0)
#define ADC1_Channel06ON ((0<<5)+0)
#define ADC1_Channel07ON ((0<<5)+0)
#define ADC1_Channel08ON ((0<<5)+0)
#define ADC1_Channel09ON ((0<<5)+0)
#define ADC1_Channel10ON ((0<<5)+0)
#define ADC1_Channel11ON ((0<<5)+0)
#define ADC1_Channel12ON ((0<<5)+0)
#define ADC1_Channel13ON ((0<<5)+0)
#define ADC1_Channel14ON ((0<<5)+0)
#define ADC1_Channel15ON ((0<<5)+0)
#define ADC1_Channel16ON ((0<<5)+0)
#define ADC1_Channel17ON ((0<<5)+0)
/*
ADC1_ChannelxON ((y<<5)+z)
x:通道数
y:是否使能 1使能 0 不使能
z:顺序 0-18
0-18个数据放在
ADC_ConvertedValue[-1]->顺序0 (z=0)
ADC_ConvertedValue[ 0]->顺序1 (z=1)
ADC_ConvertedValue[ 1]->顺序2 (z=2)
...
...



*/
/***ADC***/
/****************************************/

#endif
