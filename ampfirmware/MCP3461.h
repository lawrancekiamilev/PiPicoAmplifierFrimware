#ifndef MCP3461_h
#define MCP3461_h



#define ADCDATA_ADDR        0x0
#define CONFIG0_ADDR        0x1
#define CONFIG1_ADDR        0x3c
#define CONFIG2_ADDR        0x3
#define CONFIG3_ADDR        0x4
#define IRQ_ADDR            0x5
#define MUX_ADDR            0x6
#define SCAN_ADDR           0x7
#define TIMER_ADDR          0x8
#define OFFSET_ADDR         0x9
#define GAIN_ADDR           0xA
#define RESERVED1_ADDR      0xB
#define RESERVED2_ADDR      0xC
#define LOCK_ADDR           0xD
#define RESERVED3_ADDR      0xE
#define CRCCFG_ADDR         0xF
#define spi_addr            0b01




#define _DEFAULT_UNLOCK     0xA5
#define _DEFAULT_LOCK       0x00
#define _LockMem            0x00
#define DEFAULT_SCAN_0      0x00  // no delay between each conversion during scan cycle
#define DEFAULT_SCAN_1      0x00
#define DEFAULT_SCAN_2      0x00




#define DEFAULT_TIMER_0     0x00  // no time interval between two consecutive scan cycles
#define DEFAULT_TIMER_1     0x00
#define DEFAULT_TIMER_2     0x00

#define DEFAULT_OFFSET_0    0x00  // no offset error digital calibration
#define DEFAULT_OFFSET_1    0x00
#define DEFAULT_OFFSET_2    0x00

#define DEFAULT_GAIN_0      0x80  // default gain of 1x
#define DEFAULT_GAIN_1      0x00
#define DEFAULT_GAIN_2      0x00

#define DEFAULT_Gain1_0     0x80  // should be set to 0x900000
#define DEFAULT_Gain1_1     0x00
#define DEFAULT_Gain1_2     0x00

#define DEFAULT_RESERVED3_0 0x00  // should be set to 0x0008 for MCP3461R
#define DEFAULT_RESERVED3_1 0x08

//Config 0

#define internalRef         0x1
#define externalRef         0x0

#define shutdownFalse       0x1
#define shutdownTrue        0x0

#define internalClckOut     0x3
#define internalClck        0x2
#define externalClck        0x0

#define iSource15u          0x3
#define iSource3_7u         0x2
#define iSource_9u          0x1
#define iSource0u           0x0 

#define conversionMode      0x3
#define StandbyMode         0x2
#define ShutDownMode        0x0

//Config 1

#define AMCLK8              0x3
#define AMCLK4              0x2
#define AMCLK2              0x1
#define AMCLK0              0x0

#define OSR98304            0xF
#define OSR81920            0xE
#define OSR49152            0xD
#define OSR40960            0xC
#define OSR24576            0xB
#define OSR20480            0xA
#define OSR16384            0x9
#define OSR8192             0x8
#define OSR4086             0x7
#define OSR2048             0x6
#define OSR1024             0x5
#define OSR512              0x4
#define OSR256              0x3
#define OSR128              0x2
#define OSR64               0x1
#define OSR32               0x0

#define OSRRESERVED         0x0


//Config 2

#define biasI2              0x3
#define biasI1              0x2
#define biasI_66            0x1
#define biasI_5             0x0

#define GainX64             0x7
#define GainX32             0x6
#define GainX16             0x5
#define GainX8              0x4
#define GainX4              0x3
#define GainX2              0x2
#define GainX1              0x1
#define GainX_333           0x0

#define autoZerotrue        0x1
#define autoZerofalse       0x0
#define Conf2Reserverd      0x1
//config 3

#define ContinuousConv      0x3
#define OneShotConvStdby    0x2
#define OneShotConvShtD     0x0

#define output32bitmode1    0x3
#define output32bitmode2    0x2
#define output32bitmode3    0x1
#define output16bitmode     0x0

#define CRCformat32         0x1
#define CRCformat16         0x0

#define CRCenabled          0x1
#define CRCdisabled         0x0

#define OffsetCalTrue       0x1
#define OffsetCalFalse      0x0

#define GainCalTrue         0x1
#define GainCalFalse        0x0

//IRQ Register
#define irqMSB              0x0
#define IRQmDAT             0x1
#define IRQ                 0x0
#define IRQRising           0x1
#define IRQFalling          0x0
#define FastcmdTrue         0x1
#define FastcmdFalse        0x0
#define conversionIntTrue   0x1
#define conversionIntFalse  0x0

//MuxReg

#define MuxChan0            0x0
#define MuxChan1            0x1
#define MuxChanVcm          0xF
#define MuxDiodeMinus       0xE
#define MuxDiodePlus        0xD
#define MuxRefMinus         0xC
#define MuxRefPlus          0xB
#define MuxAVDD             0x9
#define MuxAGND             0x8
#define MuxChanOFFSET       0x0
#define statusGood          0x1



#define AdcChan0            0
#define AdcChan1            1
#define RefPlus             2
#define RefMinus            3
#define AVDD                4
#define AGND                5
#define TempDiodP           6
#define TempDiodM           7
#define ADCVCM              8  //should be 1.2V +/- 2%



//fast commands
#define StartConversion     0x68
#define Standby             0x6c
#define ShutDown            0x70
#define FullShutDown        0x74
#define FullReset           0x78
#define StaticRead          0x1
#define incRead             0x3
#define incWrite            0x2
#define address             0x1

#define DEFAULT_CRCCFG_0    0x00  
#define DEFAULT_CRCCFG_1    0x00


extern uint8_t config0;
extern uint8_t config1;
extern uint8_t config2;
extern uint8_t config3;
extern uint8_t IRQReg;




extern volatile bool IRQFLAG;
extern bool VCMStatus;
extern bool porStatus;
typedef struct{
    uint16_t conversion;
    bool memSafe;
} conversionResult;

extern conversionResult Chan0;
extern conversionResult Chan1;
extern conversionResult Temp;
extern conversionResult Offset;
extern conversionResult gainError;

extern float gain;
extern uint16_t offset;

extern int VcmHigh;
extern int VcmLow;
// Function declarations
void IRQTrig(uint gpio, uint32_t events); // interupt for IRQ pin

//chip select functions
void CsDisable();
void CsEnable();

//adc Core function
int8_t writeADC(const uint8_t *command, size_t length);
void setUpAdc(int chP, int chN);
conversionResult getSingleConv();

// adc commands
void fastCommand(uint8_t type, uint8_t reg);// handles static read, incrimental write, and incrimental read
void reset();//resets adc registers to defualt
void standBy();
void ADCshutDown();
void startConversion();
void lockADC(bool lock);// locks adc memory if bool set true
void ReadRegisters();// reads all writable registers starting at conf0
float getADCtemp();
void setOffsetError();
void setGainError();
void setMux(int chP, int chN);
bool checkADCvcm();

//adc check functions
bool checkStatus(uint8_t status);

//suport functions
float getVoltage(int32_t Conversion);
uint8_t getMux(int chP, int chN);
void printBinary(uint8_t byte);
float round_to_sig_figs(float value, int sig_figs);



//setFunctions
void ConfigADC();
void setConfig0(uint8_t Vref_Sel,uint8_t shutdown ,uint8_t CLCK_SEL, uint8_t Gain_SEL, uint8_t ADC_MODE);
void setConfig1(uint8_t AMCLK, uint8_t OSR);
void setConfig2(uint8_t BiasCurrent, uint8_t Gain, uint8_t MuxZeroing, uint8_t RefZeroing);
void setConfig3(uint8_t Conv_Mode, uint8_t DataFormat, uint8_t CRCFormat, uint8_t CRC, uint8_t OFFCAL, uint8_t GainCal);
void setIRQREG(uint8_t IRQMode, uint8_t IRQtrigger, uint8_t FastCmd, uint8_t ConversionTrig);
#endif