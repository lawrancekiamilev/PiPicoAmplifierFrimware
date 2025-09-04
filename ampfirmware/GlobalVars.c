#include "PinDeff.h"
#include "MCP3461.h"
#include "AD5204.h"
#include "I2cSlave.h"
#include "Calibration.h"

// i2c Vars
bool AmpPwrStatus = false;
bool ADCPwrStatus = false;

bool AmpON = true;
bool AmpOFF = false;
bool CalibrateAmps = false;

bool AdcShutDown = false;
bool AdcTurnOn = true;
bool resetADC = false;
bool startSample = true;

bool StopSystemDraw = false; 
bool StartSystemDraw = false;

bool testMode = false;

bool AmpCalStatus = false;
bool TempStatus = false;
bool prevTempStatus = false;
bool OffsetStatus = true;
volatile bool Savebias = false;
volatile uint16_t biasToSave = 0;

uint8_t rx_buffer[3] = {0x0};
volatile uint8_t rx_index = 0;

uint8_t tx_buffer[27] = {0x0}; 

volatile bool IRQFLAG = false;
uint8_t ampPos = 0x0;

// adc Vars
bool VCMStatus = true;
bool porStatus = false;
const float vref = 2.4; // 2.4V internal reference
/*
int VcmHigh = 19660; // ((1.2/vref)*32767)+((1.2/vref)*32767)*0.02
int VcmLow = 16056; // ((1.2/vref)*32767)-((1.2/vref)*32767)*0.02
*/

int VcmHigh = 12153; // ((1.2/vref)*32767)+((1.2/vref)*32767)*0.02
int VcmLow = 11677; // ((1.2/vref)*32767)-((1.2/vref)*32767)*0.02


conversionResult Temp;
conversionResult Offset;
conversionResult gainError;


//setup vars
mutex_t my_mutex;
volatile float ADCtemp = 0.0;


volatile bool populateTxBufferr = false;
volatile bool TxBufferPopulated = true;


int biasTarget = 12288;
