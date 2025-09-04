#include "PinDeff.h"

const uint8_t MuxChan[4] = {0x0, 0x1, 0x2, 0x3};


void IterateMux(int chan){
    uint8_t bit0 = MuxChan[chan] & 0x1;
    uint8_t bit1 = (MuxChan[chan] >> 1) & 0x1;

    gpio_put(pin_mux0_a0, bit0);
    gpio_put(pin_mux1_a0, bit0);

    gpio_put(pin_mux0_a1, bit1);
    gpio_put(pin_mux1_a1, bit1);

}