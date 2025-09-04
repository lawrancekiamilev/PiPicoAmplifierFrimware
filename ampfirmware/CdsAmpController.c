#include "PinDeff.h"
#include "MCP3461.h"
#include "AD5204.h"
#include "I2cSlave.h"
#include "Calibration.h"

conversionResult Chan0;
conversionResult Chan1;
volatile bool core1ShouldPause = false;
volatile bool core1Paused = false;
void resetTX()
{
    for (int i = 0; i < 27; i++)
    {
        tx_buffer[i] = 0x0;
    }
}
void printTXbuffer()
{
    for (int i = 1; i < 27; i += 2)
    {
        if (i + 1 < 27)
        {                                   // Ensure there is a pair to combine
            uint8_t msb = tx_buffer[i];     // Odd-indexed byte (MSB)
            uint8_t lsb = tx_buffer[i + 1]; // Even-indexed byte (LSB)
            uint16_t txValue = ((uint16_t)msb << 8) | lsb;
            printf("%i, ", txValue);
        }
    }
}
void populateTXBuffer()
{
    for (int i = 0; i < 8; i++)
    {
        tx_buffer[1 + i] = IntTemps[i];
    }
    uint16_t adcTemp = (uint16_t)roundf(ADCtemp * 100.0);
    tx_buffer[9] = (adcTemp >> 8) & 0xFF;
    tx_buffer[10] = adcTemp & 0xFF;
    for (int i = 0; i < 16; i++)
    {
        tx_buffer[11 + i] = voltBuffer[i];
    }
}

void core1_entry()
{
    setupCore1();
    while (true)
    {   
        i2c_slave_polling();
        checkSystemTemp();
        if (populateTxBufferr)
        {
            uint8_t statusByte = 0x0;
            checkSystemTemp();
            resetTX();
            mutex_enter_blocking(&my_mutex);
            populateTxBufferr = false;
            mutex_exit(&my_mutex);
            populateTempBuffer();
            statusByte |= (ampPos << 6);       // Set bits 6, and 7 with 0x3
            statusByte |= (AmpPwrStatus << 5); // Set bit 4 based on AmpPwrStatus
            statusByte |= (ADCPwrStatus << 4);
            statusByte |= (VCMStatus << 3);    // Set bit 3 based on porStatus
            statusByte |= (AmpCalStatus << 2);  // Set bit 2 based on CalibrateAmps
            statusByte |= (OffsetStatus << 1); // Set bit 1 based on OffsetStatus
            statusByte |= (TempStatus << 0);   // Set bit 0 based on TempStatus
            tx_buffer[0] = statusByte;
            
            populateTXBuffer();
            triggerTestIRQ();

            mutex_enter_blocking(&my_mutex);
            TxBufferPopulated = true;
            mutex_exit(&my_mutex);
        }
    }
}

int main()
{
    setup();
    ConfigADC();
    mutex_enter_blocking(&my_mutex);
    populateTxBufferr = true;
    mutex_exit(&my_mutex);
    while (true)
    {
        while (!TxBufferPopulated)
        {
            tight_loop_contents();
        }
        mutex_enter_blocking(&my_mutex);
        TxBufferPopulated = false;
        mutex_exit(&my_mutex);
        if (AmpON)
        {
            gpio_put(pin_nPD, 1);
            gpio_put(pin_nPD_LED, 1);
            mutex_enter_blocking(&my_mutex);
            AmpON = false;
            AmpPwrStatus = true;
            mutex_exit(&my_mutex);
        }

        if (AmpOFF)
        {
            gpio_put(pin_nPD, 0);
            gpio_put(pin_nPD_LED, 0);
            mutex_enter_blocking(&my_mutex);
            AmpOFF = false;
            AmpPwrStatus = false;
            mutex_exit(&my_mutex);
        }
        if (AdcTurnOn)
        {
            setAllPots(127);
            setUpAdc(AdcChan0, RefMinus);
            bool VCMStat = checkADCvcm();
            mutex_enter_blocking(&my_mutex);
            AdcTurnOn = false;
            ADCPwrStatus = true;
            VCMStatus = VCMStat;
            mutex_exit(&my_mutex);
            sleep_ms(10);
        }

        if (CalibrateAmps)
        {
            CalabrateChannels();
            mutex_enter_blocking(&my_mutex);
            CalibrateAmps = false;
            mutex_exit(&my_mutex);
        }


        if (AdcShutDown)
        {
            mutex_enter_blocking(&my_mutex);
            AdcShutDown = false;
            ADCPwrStatus = false;
            mutex_exit(&my_mutex);
            reset();
            ADCshutDown();
        }

        if (resetADC)
        {
            setUpAdc(AdcChan0, RefMinus);
            bool VCMStat = checkADCvcm();
            mutex_enter_blocking(&my_mutex);
            resetADC = false;
            VCMStatus = VCMStat;
            mutex_exit(&my_mutex);
        }

        if (startSample && ADCPwrStatus)
        {
            CheckChanVoltages();
        }

        if (testMode)
        {
            mutex_enter_blocking(&my_mutex);
            testMode = false;
            populateTxBufferr = true;
            mutex_exit(&my_mutex);
            runTroughPots();
        }

        if (IRQFLAG)
        {
            CalabrateChannels();
        }

        if (TempStatus && !prevTempStatus)
        {
            if (!(gpio_get_out_level(pin_nPD)))
            {
                gpio_put(pin_nPD, 1);
                gpio_put(pin_shtdwn_LED, 1);
                prevTempStatus = true;
            }
        }
        if (!TempStatus && prevTempStatus)
        {
            if (gpio_get_out_level(pin_nPD))
            {
                gpio_put(pin_nPD, 0);
                gpio_put(pin_shtdwn_LED, 0);
                prevTempStatus = false;
            }
        }
        if (ADCPwrStatus)
        {
            float temp = getADCtemp();
            bool VCMStat = checkADCvcm();
            mutex_enter_blocking(&my_mutex);
            ADCtemp = temp;
            VCMStatus = VCMStat;
            mutex_exit(&my_mutex);
        }
        if(!ADCPwrStatus|| !startSample){
            sleep_ms(1000);
        }
        

        mutex_enter_blocking(&my_mutex);
        populateTxBufferr = true;
        mutex_exit(&my_mutex);
    }
}
