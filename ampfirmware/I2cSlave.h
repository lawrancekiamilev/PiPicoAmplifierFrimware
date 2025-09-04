#ifndef I2cSlave_h
#define I2cSlave_h

#define I2C_PORT                    i2c0
#define pin_i2c_id0                 13      // ID0 pin for I2C address
#define pin_i2c_id1                 14      // ID1 pin for I2C address
#define pin_SDA                     0       // SDA I2C pin
#define pin_SCL                     1       // SCL I2C pin


#define I2C_address0                0x55    // i2c address 0
#define I2C_address1                0x44    // i2c address 0
#define I2C_address2                0x33    // i2c address 0
#define I2C_address3                0x22    // i2c address 0

extern uint8_t ampPos;
extern uint8_t rx_buffer[3];
extern uint8_t tx_buffer[27];
extern volatile uint8_t rx_index;

extern volatile bool populateTxBufferr;
extern volatile bool TxBufferPopulated;

extern bool AmpPwrStatus;
extern bool ADCPwrStatus;

extern bool AmpON;
extern bool AmpOFF;
extern bool CalibrateAmps;

extern bool AdcShutDown;
extern bool AdcTurnOn;
extern bool resetADC;
extern bool startSample;

extern bool StopSystemDraw; 
extern bool StartSystemDraw;

extern bool testMode;

extern bool AmpCalStatus;
extern bool TempStatus;
extern bool prevTempStatus;
extern bool OffsetStatus;



void i2c_slave_polling();
uint8_t findAdress();
void initI2Cslave();

#endif