#include "PinDeff.h"
#include "MCP3461.h"
#include "AD5204.h"
#include "Calibration.h"
#include "I2cSlave.h"


const int tolerance = 16;
uint16_t volatile Voltages[8] = {0x0};

//external 3.3v Refrence
const int breakPoint1 = 11649;
const int breakPoint2 = 11409;
const int breakPoint3 = 11175;
const int breakPoint4 = 10949;
const int breakPoint5 = 10729;
const int breakPoint6 = 10516;
const int breakPoint7 = 10307;
const int breakPoint8 = 10104;
const int breakPoint9 = 9905;
const int breakPoint10 = 9710;
const int breakPoint11 = 9519;
const int breakPoint12 = 9331;
const int breakPoint13 = 9146;
const int breakPoint14 = 8964;
const int breakPoint15 = 8783;

/*
//internal 2.4V reffrence
const int breakPoint1 = 16140;
const int breakPoint2 = 15809;
const int breakPoint3 = 15487;
const int breakPoint4 = 15176;
const int breakPoint5 = 14872;
const int breakPoint6 = 14577;
const int breakPoint7 = 14288;
const int breakPoint8 = 13988;
const int breakPoint9 = 13730;
const int breakPoint10 = 13459;
const int breakPoint11 = 13193;
const int breakPoint12 = 12931;
const int breakPoint13 = 12671;
const int breakPoint14 = 12415;
const int breakPoint15 = 12160;
*/

int Y1[8] = {0};
int Y2[8] = {0};

float Slope[8] = {0};


volatile uint8_t voltBuffer[16] = {0x0};


void CheckChanVoltages()
{
    conversionResult Chan;
    for (int i = 0; i < 4; i++)
    {
        IterateMux(i);
        setMux(AdcChan0, RefMinus);
        Chan = getSingleConv();
        while (!Chan.memSafe)
        {
            Chan = getSingleConv();
        }
        Voltages[i] = Chan.conversion;
        setMux(AdcChan1, RefMinus);
        Chan = getSingleConv();
        while (!Chan.memSafe)
        {
            Chan = getSingleConv();
        }
        Voltages[i + 4] = Chan.conversion;
    }
    for (int i = 0; i < 8; i++)
    {
        voltBuffer[2 * i] = (Voltages[i] >> 8) & 0xFF;
        voltBuffer[2 * i + 1] = Voltages[i] & 0xFF;
    }
    for (int j = 0; j < 8; j++)
    {
        if (!(Voltages[j] >= (biasTarget + tolerance) || !(Voltages[j] < (biasTarget - tolerance))))
        {
            PulseBiasLED(1,1000);
            mutex_enter_blocking(&my_mutex);
            OffsetStatus = true;
            mutex_exit(&my_mutex);
            return;
        }
    }
    mutex_enter_blocking(&my_mutex);
    OffsetStatus = false;
    mutex_exit(&my_mutex);
    PulseBiasLED(0, 1000);
}

void CalabrateChannels()
{
    PulseBiasLED(0.1, 10);
    mutex_enter_blocking(&my_mutex);
    AmpCalStatus = true;
    mutex_exit(&my_mutex);

    if (biasTarget < 0 || biasTarget > 0xFFFF) {
        PulseBiasLED(0, 1000);
        return;
    }
    printf("Calibrating channels with bias target: %d\n", biasTarget);
    const int pot_step_size = 16;
    const int total_steps = 256;
    const int segment_count = total_steps / pot_step_size;

    int finalPotValues[8] = {-1, -1, -1, -1, -1, -1, -1, -1}; // -1 means "not set"

    for (int segment = 0; segment < segment_count; ++segment) {
        int pot_start = segment * pot_step_size;
        int pot_end = pot_start + pot_step_size;

        SetSlopeOffset(pot_start, pot_end); // Sets Y1, Y2, Slope for all channels
        printf("Segment %d: pot range [%d–%d]\n", segment, pot_start, pot_end);
        for (int i = 0; i < 8; i++) {
            printf("  Chan %d: Y1 = %d, Y2 = %d (range: [%d–%d])\n",i, Y1[i], Y2[i], (int)fminf(Y1[i], Y2[i]), (int)fmaxf(Y1[i], Y2[i]));
            if (finalPotValues[i] != -1) continue; // Already found best segment

            float v_start = (float)Y1[i];
            float v_end = (float)Y2[i];
            float v_min = fminf(v_start, v_end);
            float v_max = fmaxf(v_start, v_end);

            if (biasTarget >= v_min && biasTarget <= v_max) {
                float potVal_f = ((biasTarget - Y1[i]) / Slope[i]);
                int potVal = (int)roundf(potVal_f) + pot_start;
                finalPotValues[i] = potVal;

                printf("Channel %d matched segment %d: Setting pot to %d\n", i, segment, potVal);
            }
        }
    }

    // Apply final values to pots
    for (int i = 0; i < 8; i++) {
        if (finalPotValues[i] != -1) {
            if (i < 4) {
                SetPot(0, i, finalPotValues[i]);
            } else {
                SetPot(1, i - 4, finalPotValues[i]);
            }
        } else {
            printf("No segment found for channel %d\n", i);
        }
    }
    mutex_enter_blocking(&my_mutex);
    AmpCalStatus = false;
    mutex_exit(&my_mutex);
    PulseBiasLED(0, 1000);
}
    




void getPotValues(int *offset, float *slope, int incriment){
    for (int i = 0; i < 8; i++)
    {
        float PotValuef = ((biasTarget - offset[i]) / slope[i]);

        int PotValue = (int)round(PotValuef)+ incriment;
        if (i < 4)
        {
            if(i == 1){continue;}
            SetPot(0, i, PotValue);
        }
        else
        {
            SetPot(1, i - 4, PotValue);
        }
    }
}


/*
void GetOffset()
{
    setAllPots(1);
    for (int i = 0; i < 4; i++)
    {
        IterateMux(i);
        setMux(AdcChan0, RefMinus);
        conversionResult converision = getSingleConv();
        while (!converision.memSafe)
        {
            converision = getSingleConv();
        }
        ampOffset[i] = converision.conversion;

        setMux(AdcChan1, RefMinus);
        converision = getSingleConv();
        while (!converision.memSafe)
        {
            converision = getSingleConv();
        }
        ampOffset[i + 4] = converision.conversion;
    }
    printf("offsets: ");
    for(int i = 0; i<8; i++){
        printf("%i, ",ampOffset[i]);
    }
    printf("\n");
}
*/

void setStepArray(int *array, int potVal)
{
    conversionResult Chan;
    setAllPots(potVal);
    for (int j = 0; j < 4; j++)
    {
        IterateMux(j);
        setMux(AdcChan0, RefMinus);
        Chan = getSingleConv();
        while (!Chan.memSafe)
        {
            Chan = getSingleConv();
        }
        array[j] = Chan.conversion;
        setMux(AdcChan1, RefMinus);
        Chan = getSingleConv();
        while (!Chan.memSafe)
        {
            Chan = getSingleConv();
        }
        array[j + 4] = Chan.conversion;
    }
}


void GetSlope(int *y1,int *y2, float *slope){
    for (int i = 0; i < 8; i++)
    {
        int deltaSlope = (y2[i] - y1[i]);
        float s = ((float)deltaSlope) / (16.0);
        slope[i] = s;
    }
}


void SetSlopeOffset(int y1PotVal, int y2PotVal)
{
    for (int i = 0; i < 2; i++)
    {
        setStepArray(Y1,y1PotVal);
        setStepArray(Y2,y2PotVal);
    }
    GetSlope(Y1,Y2,Slope);
}

void runTroughPots()
{
    conversionResult conversion;
    for (int i = 0; i < 255; i++)
    {
        while (!TxBufferPopulated)
        {
            tight_loop_contents();
        }
        mutex_enter_blocking(&my_mutex);
        TxBufferPopulated = false;
        mutex_exit(&my_mutex);
        setAllPots(i);
        sleep_us(500);
        for (int j = 0; j < 4; j++)
        {
            IterateMux(j);
            setMux(AdcChan0, RefMinus);
            conversion = getSingleConv();
            while (!conversion.memSafe)
            {
                conversion = getSingleConv();
            }
            Voltages[j] = conversion.conversion;
            setMux(AdcChan1, RefMinus);
            conversion = getSingleConv();
            while (!conversion.memSafe)
            {
                conversion = getSingleConv();
            }
            Voltages[j + 4] = conversion.conversion;
        }
        for (int i = 0; i < 8; i++)
        {
            voltBuffer[2 * i] = (Voltages[i] >> 8) & 0xFF;
            voltBuffer[2 * i + 1] = Voltages[i] & 0xFF;
        }
        mutex_enter_blocking(&my_mutex);
        populateTxBufferr = true;
        mutex_exit(&my_mutex);
    }
}

void triggerTestIRQ()
{
    gpio_put(TestCRQpin, 0);
    sleep_us(2);
    gpio_put(TestCRQpin, 1);
}


void printArray(void *array, size_t element_size, size_t length)
{
    printf("{");
    for (size_t i = 0; i < length; i++)
    {
        if (element_size == sizeof(int))
        {
            printf("%d, ", *((int *)array + i));
        }
        else if (element_size == sizeof(float))
        {
            printf("%f, ", *((float *)array + i));
        }
        else
        {
            printf("Unknown type\n");
            break;
        }
    }
    printf("}\n");
}
void printValus(float *slope, int *offset){
    printf("slope: ");
    for(int i=0; i<8; i++){
        printf("%f, ",slope[i]);
    }
    printf("\n");
    printf("offsets: ");
    for(int i=0; i<8; i++){
        printf("%i, ",offset[i]);
    }
    printf("\n");
}

void PulseBiasLED(float DutyCycle, float frequency) {
    // Validate inputs
    if (DutyCycle < 0.0f) DutyCycle = 0.0f;
    if (DutyCycle > 1.0f) DutyCycle = 1.0f;
    if (frequency <= 0.0f) frequency = 1.0f;  // Prevent division by zero
    
    // Calculate duty cycle in counts (20,000 count wrap)
    uint16_t DC = (uint16_t)roundf(65535.0f * DutyCycle);
    
    // Calculate clock divider based on desired frequency
    // System clock is 125MHz, we want: 125,000,000 / (20,000 * frequency)
    float divider = 125000000.0f / (65535.0f * frequency);
    
    // Setup PWM
    gpio_set_function(pin_bias_LED, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin_bias_LED);
    uint chan_num = pwm_gpio_to_channel(pin_bias_LED);
    
    // Configure PWM
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, divider);
    pwm_config_set_wrap(&config, 0xffff);  // 20,000 in hex
    pwm_init(slice_num, &config, true);
    
    // Set duty cycle
    pwm_set_chan_level(slice_num, chan_num, DC);
    
    // Enable PWM
    pwm_set_enabled(slice_num, true);
}


