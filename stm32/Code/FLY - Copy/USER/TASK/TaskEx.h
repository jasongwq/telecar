#ifdef __cplusplus
       extern "C" {
#endif
#include "data_transfer.h"
int task_pwm_ex(void)
{
    _SS
    Ex_Init();
    while (1)
    {
        if (Ex_ON_OFF.Status)
        {
            WaitX(EX_BaudRate);
            Send.Status = 1;
            Data_Exchange();
        }
        if (Ex_ON_OFF.Senser)
        {
            WaitX(EX_BaudRate);
            Send.Senser = 1;
            Data_Exchange();
        }
        if (Ex_ON_OFF.RCData)
        {
            WaitX(EX_BaudRate);
            Data_Exchange();
        }
        if (Ex_ON_OFF.MotoPwm)
        {
            WaitX(EX_BaudRate);
            Send.MotoPwm = 1;
            Data_Exchange();
        }
        if (Ex_ON_OFF.DataF1)
        {
            WaitX(EX_BaudRate);
            Send.DataF1 = 1;
            Data_Exchange();
        }
        if (Ex_ON_OFF.DataF2)
        {
            WaitX(EX_BaudRate);
            Send.DataF2 = 1;
            Data_Exchange();
        }
        if (Ex_ON_OFF.DataF3)
        {
            WaitX(EX_BaudRate);
            Send.DataF3 = 1;
            Data_Exchange();
        }
        if (Ex_ON_OFF.DataF4)
        {
            WaitX(EX_BaudRate);
            Send.DataF4 = 1;
            Data_Exchange();
        }
        WaitX(EX_BaudRate);
        Data_Exchange();
    }
    _EE
}
#ifdef __cplusplus
        }
#endif
