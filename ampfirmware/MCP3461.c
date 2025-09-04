#include "PinDeff.h"
#include "MCP3461.h"


//chip select functions
void CsEnable(){
    gpio_put(pin_nCS_adc, 0);
    sleep_us(10);
}

void CsDisable(){
    sleep_us(10);
    gpio_put(pin_nCS_adc, 1);
    sleep_us(10);
}


//adc Core function
int8_t writeADC(const uint8_t *command, size_t length) {
    uint8_t rxBuffer[length];
    spi_write_read_blocking(SPI_PORT, command, rxBuffer, length);
    return rxBuffer[0];
}


void setUpAdc(int chP, int chN) {
    uint8_t mux_data = getMux(chP,chN);
    uint8_t configData[] = {
        config0,
        config1,
        config2,
        config3,
        IRQReg,
        mux_data,
        DEFAULT_SCAN_0, DEFAULT_SCAN_1, DEFAULT_SCAN_2,
        DEFAULT_TIMER_0, DEFAULT_TIMER_1, DEFAULT_TIMER_2,
        DEFAULT_OFFSET_0, DEFAULT_OFFSET_1, DEFAULT_OFFSET_2,
        DEFAULT_Gain1_0, DEFAULT_Gain1_1, DEFAULT_Gain1_2
    };

    reset();
    lockADC(false);
    CsEnable();
    fastCommand(incWrite, CONFIG0_ADDR);
    uint8_t status = writeADC(configData, sizeof(configData));
    CsDisable();
    lockADC(true);
    startConversion();
    setOffsetError();
    setGainError();
}




conversionResult getSingleConv() {
    sleep_us(10);
    uint8_t buffer[4] = {0};
    conversionResult adc_data = {0, false};  // Initialize with default values
    startConversion();
    CsEnable();
    fastCommand(StaticRead, ADCDATA_ADDR);  // Point to ADC data address
    spi_read_blocking(SPI_PORT, 0, buffer, sizeof(buffer));  // Read ADC value
    CsDisable();

    // Validate buffer data for consistency
    if (((buffer[2] << 8) | buffer[3]) == ((buffer[0] << 8) | buffer[1])) {
        adc_data.memSafe = true;
        adc_data.conversion = ((buffer[2] << 8) | (buffer[3]));
    } else {
        adc_data.memSafe = false;
        adc_data.conversion = 0;  // Reset to a default safe value if data is inconsistent
    }

    return adc_data;
}



// adc commands


   
void fastCommand(uint8_t type, uint8_t reg) {
    uint8_t cmd;
    bool statGood = false;
    int attempts = 0;  // Counter to prevent infinite loops

    switch(type) {
        case StaticRead:
            cmd = (address << 6) | (reg << 2) | StaticRead;
            while (!statGood && attempts < 10) {  // Limit attempts to 10
                uint8_t status = writeADC(&cmd, 1);
                statGood = checkStatus(status);
                attempts++;
            }
            break;
        case incRead:
            cmd = (address << 6) | (reg << 2) | incRead;
            while (!statGood && attempts < 10) {
                uint8_t status = writeADC(&cmd, 1);
                statGood = checkStatus(status);
                attempts++;
            }
            break;
        case incWrite:
            cmd = (address << 6) | (reg << 2) | incWrite;
            while (!statGood && attempts < 10) {
                uint8_t status = writeADC(&cmd, 1);
                statGood = checkStatus(status);
                attempts++;
            }
            break;
    }

    if (!statGood) {
        printf("fastCommand: Failed to complete command after 10 attempts\n");
    }
}


void reset(){
    uint8_t cmd = FullReset;
    bool statGood = false;
    uint8_t status;
    while(!statGood){
        CsEnable();
        status = writeADC(&cmd, 1);
        CsDisable();
        statGood = checkStatus(status);
    }
    if(status&1 == 0){
        mutex_enter_blocking(&my_mutex);
        porStatus = true;
        mutex_exit(&my_mutex);
    }
    else{
        mutex_enter_blocking(&my_mutex);
        porStatus = false;
        mutex_exit(&my_mutex);
    }
}


void standBy(){
    uint8_t cmd = Standby;
    bool statGood = false;
    while(!statGood){
        CsEnable();
        uint8_t status = writeADC(&cmd, 1);
        CsDisable();
        statGood = checkStatus(status);
    }
}
void ADCshutDown(){
    uint8_t cmd = ShutDown;
    bool statGood = false;
    reset();
    while(!statGood){
        CsEnable();
        uint8_t status = writeADC(&cmd, 1);
        CsDisable();
        statGood = checkStatus(status);
    }
}

void startConversion(){
    uint8_t cmd = StartConversion;
    bool statGood = false;
    bool ConveriosnStart = false;
    while(!statGood){
        CsEnable();
        uint8_t status = writeADC(&cmd, 1);
        CsDisable();
        statGood = checkStatus(status);
    }
    // Wait for IRQFLAG signal
    while(!IRQFLAG) {
        tight_loop_contents();
    }
    IRQFLAG = false;
    while(!IRQFLAG) {
        tight_loop_contents();
    }
    IRQFLAG = false; 

}


void lockADC(bool lock){
    bool statGood = false;
    uint8_t status;
    if(lock){
        uint8_t cmd = _DEFAULT_LOCK;
        CsEnable();
        fastCommand(incWrite, LOCK_ADDR);
        status = writeADC(&cmd, 1);
        CsDisable();
    }
    else{
        uint8_t cmd = _DEFAULT_UNLOCK;
        CsEnable();
        fastCommand(incWrite, LOCK_ADDR);
        status = writeADC(&cmd, 1);
        CsDisable();

    }
}


void ReadRegisters(){
    uint8_t buffer[22] = {0};
    CsEnable();
    fastCommand(incRead, CONFIG0_ADDR);
    spi_read_blocking(SPI_PORT, 0, buffer, sizeof(buffer));
    CsDisable();
    for(int i=0; i<sizeof(buffer); i++){
        printf("Reg:%i ::: Data:%i\n",i ,buffer[i] );
    }
}


float getADCtemp(){
    bool readingGood = false;
    uint16_t tmpValue; 
    setMux(TempDiodP,TempDiodM);
    while(!readingGood){
        Temp = getSingleConv();
        readingGood = Temp.memSafe;
        tmpValue = Temp.conversion;
    }
    float temp = 0.102646 * tmpValue * vref-269.13;
    return round_to_sig_figs(temp,2);
}


void setOffsetError(){
    bool readingGood = false;
    uint16_t offsetValue;
    uint8_t offsetValueBuffer[3] = {0};
    setMux(RefMinus,RefMinus);
    while(!readingGood){
        Offset = getSingleConv();
        readingGood = Offset.memSafe;
        offsetValue = Offset.conversion;
    }
    offsetValueBuffer[0] = (offsetValue >> 8) & 0xff;
    offsetValueBuffer[1] =  offsetValue & 0xff;
    lockADC(false);

    CsEnable();
    fastCommand(incWrite, OFFSET_ADDR);
    uint8_t status = writeADC(offsetValueBuffer, sizeof(offsetValueBuffer));
    CsDisable();
    lockADC(true);
}

void setGainError(){
    bool readingGood = false;
    uint8_t gainErrorBuffer[3] = {0};
    uint8_t config2ValBuffer[1] = {0};
    CsEnable();
    fastCommand(StaticRead,CONFIG2_ADDR);
    uint8_t config2Val = spi_read_blocking(SPI_PORT, 0, config2ValBuffer, 1);
    CsDisable();
    uint8_t gain = (config2ValBuffer[0]>>3) & 0x7;
    switch(gain){
        case GainX1: gainErrorBuffer[0] = 0x80; break;
        case GainX2: gainErrorBuffer[0] = 0x40; break;
        case GainX4: gainErrorBuffer[0] = 0x20; break;
        case GainX8: gainErrorBuffer[0] = 0x10; break;
        case GainX16: gainErrorBuffer[0] = 0x8; break;
        case GainX32: gainErrorBuffer[0] = 0x4; break;
        case GainX64: gainErrorBuffer[0] = 0x2; break;
    }
    lockADC(false);
    CsEnable();
    fastCommand(incWrite, GAIN_ADDR);
    uint8_t status = writeADC(gainErrorBuffer, sizeof(gainErrorBuffer));
    CsDisable();
    lockADC(true);
}


void setMux(int chP, int chN){
    bool readingGood = false;
    uint8_t mux = getMux(chP,chN);
    lockADC(false);
    CsEnable();
    fastCommand(incWrite, MUX_ADDR);
    uint8_t status = writeADC(&mux, 1);
    CsDisable();
    lockADC(true);
}

bool checkADCvcm(){
    conversionResult VCM;
    bool readingGood = false;
    uint16_t VCMval = 0x0;
    setMux(ADCVCM,AGND);
    while(!readingGood){
        VCM = getSingleConv();
        readingGood = VCM.memSafe;
        VCMval = VCM.conversion;
    }
    
    if(VCMval >= VcmLow && VCMval <= VcmHigh){
        return true;
    }
    else{
        return false;
    }

}


//Check functions



bool checkStatus(uint8_t status){
    if(status>>4 == statusGood){
        return true;
    }
    else{
        return false;
    }
}


//suport functions



float getVoltage(int32_t Conversion){
    float value = (3.3/32767) * Conversion;
    return round_to_sig_figs(value, 6);//round to six sig figs
}



uint8_t getMux(int chP, int chN){
    uint8_t muxP, muxN;
    uint8_t cmd = Standby;
    switch (chP) {
        case 0: muxP = MuxChan0; break;
        case 1: muxP = MuxChan1; break;
        case 2: muxP = MuxRefPlus; break;
        case 3: muxP = MuxRefMinus; break;
        case 4: muxP = MuxAVDD; break;
        case 5: muxP = MuxAGND; break;
        case 6: muxP = MuxDiodePlus; break;
        case 7: muxP = MuxDiodeMinus; break;
        case 8: muxP = MuxChanVcm; break;
        default: muxP = MuxChan0; break;
    }

    switch (chN) {
        case 0: muxN = MuxChan0; break;
        case 1: muxN = MuxChan1; break;
        case 2: muxN = MuxRefPlus; break;
        case 3: muxN = MuxRefMinus; break;
        case 4: muxN = MuxAVDD; break;
        case 5: muxN = MuxAGND; break;
        case 6: muxN = MuxDiodePlus; break;
        case 7: muxN = MuxDiodeMinus; break;
        case 8: muxN = MuxChanVcm; break;
        default: muxN = MuxChanVcm; break;
    }
    uint8_t mux_data = (muxP << 4) | muxN;

    return mux_data;

}


void printBinary(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
    printf("\n");
}


float round_to_sig_figs(float value, int sig_figs) {
    if (value == 0) return 0;
    
    // Determine the order of magnitude of the value
    float scale = pow(10, sig_figs - 1 - (int)floor(log10(fabs(value))));
    
    // Scale, round, then unscale to achieve the desired precision
    return round(value * scale) / scale;
}



