#include "PinDeff.h"
#include "I2cSlave.h"

volatile float Temps[4] = {0.0};
volatile uint8_t IntTemps[8] = {0x0};
const float MaxTemp = 98.0;
const float RT0 = 10000.0;
const float B = 3435.0;
const float VCC = 3.3;
const float T0 = 298.15;


void checkSystemTemp(){
    for( int i=0; i<4; i++){
        adc_select_input(i);
        uint16_t raw_value = adc_read();
        float voltage = (VCC/4085.0)*raw_value;
        float VR = VCC- voltage;
        float RT= voltage/ (VR/RT0);
        float ln = log(RT/RT0);
        Temps[i] = ((1/((ln/B)+ (1/T0))) -273.15);
    }
    if(Temps[0] <= MaxTemp || Temps[1] <= MaxTemp || Temps[2] <= MaxTemp || Temps[3] <= MaxTemp && TempStatus){
        mutex_enter_blocking(&my_mutex);
        TempStatus = false;
        mutex_exit(&my_mutex);
        gpio_put(pin_shtdwn_LED, 0);
    }
    else if(Temps[0] > MaxTemp || Temps[1] > MaxTemp || Temps[2] > MaxTemp || Temps[3] > MaxTemp && !TempStatus){
        mutex_enter_blocking(&my_mutex);
        TempStatus = true;
        mutex_exit(&my_mutex);
        gpio_put(pin_shtdwn_LED, 1);
    }

}

void populateTempBuffer(){
    for(int i = 0; i<4; i++){
        uint16_t temp16bit = ((uint16_t)roundf(Temps[i]*100.0));
        IntTemps[2 * i] = (temp16bit >> 8)& 0xFF;
        IntTemps[2 * i + 1]   =  temp16bit & 0xFF;
    }
}