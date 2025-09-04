#ifndef Calibration_h
#define Calibration_h
extern volatile uint8_t voltBuffer[16];
extern int biasTarget;
extern volatile uint16_t Voltages[8];
void CalabrateChannels();
void GetSlope(int *y1,int *y2, float *slope);
void SetSlopeOffset(int y1PotVal, int y2PotVal);
void printArray(void *array,size_t element_size, size_t length);
void GetOffset();
void runTroughPots();
void triggerTestIRQ();
void setStepArray(int *array, int potVal);
void getPotValues(int *offset, float *slope, int incriment);
void printValus(float *slope, int *offset);
void PulseBiasLED(float DutyCycle, float frequency);
#endif