#define Left                         0x10 //43  
#define MaskLeft                   ( 0x18 | 0x80) //43  
#define Right                        0x18 //43
#define MaskRight                    0x18 //43  
#define Stop                         0x84
#define MaskStop                     0x84
#define Skid                         0x80 //70  
#define MaskSkid                     0x80 //70  
#define RemoteControlSpeed           0x20 //5   
#define MaskRemoteControlSpeed       0x20 //5   
#define RemoteControlRun             0x04 //2   
#define MaskRemoteControlRun         0x04 //2   
#define RemoteControlRunH            0x06 //210 
#define MaskRemoteControlRunH     (  0x07 | 0xC0) //210 
#define RemoteControlRunM            0x05 //210 
#define MaskRemoteControlRunM     (  0x07 | 0xC0) //210 
#define RemoteControlRunL            0x04 //210 
#define MaskRemoteControlRunL     (  0x07 | 0xC0) //210 
#define RemoteControlBack            0x03 //70  
#define MaskRemoteControlBack        ( 0x07 | 0x80) //70  
#define ManualControlRun             0x40 //6   
#define MaskManualControlRun         0x40 //6   
#define ManualControlRunH            0x42 //610 
#define MaskManualControlRunH        0x42 //610 
#define ManualControlRunM            0x41 //610 
#define MaskManualControlRunM        0x41 //610 
#define ManualControlRunL            0x40 //610 
#define MaskManualControlRunL        0x40 //610 
#define ManualControlBack            0x83 //710 
#define MaskManualControlBack        0x83 //710 
#define ProofreadingFrequency        0x82//710 
#define MaskProofreadingFrequency    0x82//710 