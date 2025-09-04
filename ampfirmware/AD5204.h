#ifndef AD5204_h
#define AD5204_h

void SetPot(int Pot, int chan, int val);
void enableCSAD5204(int Pot);
void disableCSAD5204(int Pot);
void ResetPots();
void setAllPots(int val);
#endif