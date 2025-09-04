#include "PinDeff.h"
#include "MCP3461.h"

uint8_t config0;
uint8_t config1;
uint8_t config2;
uint8_t config3;
uint8_t IRQReg;
float gain;

void ConfigADC(){
    setConfig0(internalRef,shutdownFalse,internalClck,iSource0u,StandbyMode);
    setConfig1(AMCLK0,OSR98304);
    setConfig2(biasI1,GainX1,autoZerofalse,autoZerofalse);
    setConfig3(OneShotConvStdby,output16bitmode,CRCformat16,CRCdisabled,OffsetCalTrue,GainCalFalse);
    setIRQREG(IRQ,IRQRising,FastcmdTrue,conversionIntTrue);

}

void setConfig0(uint8_t Vref_Sel,uint8_t shutdown ,uint8_t CLCK_SEL, uint8_t Gain_SEL, uint8_t ADC_MODE){
    config0 = (Vref_Sel<<7)|(shutdown<<6)|(CLCK_SEL<<4)|(Gain_SEL<<2)|ADC_MODE;
    switch(Gain_SEL){
        case(GainX64):gain = 64.0;break;
        case(GainX32):gain = 32.0;break;
        case(GainX16):gain = 16.0;break;
        case(GainX8):gain =  8.0;break;
        case(GainX4):gain =  4.0;break;
        case(GainX2):gain =  2.0;break;
        case(GainX1):gain =  1.0;break;
        case(GainX_333):gain=.3333333;break;
    }
}

void setConfig1(uint8_t AMCLK, uint8_t OSR){
    config1 = (AMCLK<<6)|(OSR<<2)|OSRRESERVED;
}
void setConfig2(uint8_t BiasCurrent, uint8_t Gain, uint8_t MuxZeroing, uint8_t RefZeroing){
    config2 = (BiasCurrent<<6)|(Gain<<3)|(MuxZeroing<<2)|(RefZeroing<<1)|Conf2Reserverd;
}
void setConfig3(uint8_t Conv_Mode, uint8_t DataFormat, uint8_t CRCFormat, uint8_t CRC, uint8_t OFFCAL, uint8_t GainCal){
    config3 = (Conv_Mode<<6)|(DataFormat<<4)|(CRCFormat<<3)|(CRC<<2)|(OFFCAL<<1)|GainCal;
}
void setIRQREG(uint8_t IRQMode, uint8_t IRQtrigger, uint8_t FastCmd, uint8_t ConversionTrig){
    IRQReg = (irqMSB<<4)|(IRQMode<<3)|(IRQtrigger<<2)|(FastCmd<<1)|ConversionTrig;
}

