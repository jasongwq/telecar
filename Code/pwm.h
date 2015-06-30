void PCA_config(void)
{
    PCA_InitTypeDef     PCA_InitStructure;

    PCA_InitStructure.PCA_Clock    = PCA_Clock_12T;      //PCA_Clock_1T, PCA_Clock_2T, PCA_Clock_4T, PCA_Clock_6T, PCA_Clock_8T, PCA_Clock_12T, PCA_Clock_Timer0_OF, PCA_Clock_ECI
    PCA_InitStructure.PCA_IoUse    = PCA_P12_P11_P10_P37;   //PCA_P12_P11_P10_P37, PCA_P34_P35_P36_P37, PCA_P24_P25_P26_P27
    PCA_InitStructure.PCA_Interrupt_Mode = DISABLE;     //ENABLE, DISABLE
    PCA_InitStructure.PCA_Polity   = PolityHigh;         //优先级设置   PolityHigh,PolityLow
    PCA_InitStructure.PCA_RUN      = DISABLE;           //ENABLE, DISABLE
    PCA_Init(PCA_Counter, &PCA_InitStructure);

    PCA_InitStructure.PCA_Mode     = PCA_Mode_PWM;      //PCA_Mode_PWM, PCA_Mode_Capture, PCA_Mode_SoftTimer, PCA_Mode_HighPulseOutput
    PCA_InitStructure.PCA_PWM_Wide = PCA_PWM_8bit;      //PCA_PWM_8bit, PCA_PWM_7bit, PCA_PWM_6bit
    PCA_InitStructure.PCA_Interrupt_Mode = DISABLE;     //PCA_Rise_Active, PCA_Fall_Active, ENABLE, DISABLE
    PCA_InitStructure.PCA_Value    = 128 << 8;          //对于PWM,高8位为PWM占空比
    PCA_Init(PCA0, &PCA_InitStructure);

    CR = 1;
}
