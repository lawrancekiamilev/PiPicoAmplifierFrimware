#include "PinDeff.h"
#include "AD5204.h"

void SetPot(int Pot, int chan, int val)
{
    uint8_t channel = (uint8_t)(chan);
    uint8_t value = (uint8_t)val;
    uint8_t CMD[2] = {0};

    CMD[0] = (channel);
    CMD[1] = (value);
    enableCSAD5204(Pot);
    spi_write_blocking(SPI_PORT, CMD, 2);
    disableCSAD5204(Pot);
}

void enableCSAD5204(int Pot)
{
    switch (Pot)
    {
    case 0:
        gpio_put(pin_nCS_DCP0, 0);
        sleep_us(15);
        break;
    case 1:
        gpio_put(pin_nCS_DCP1, 0);
        sleep_us(15);
        break;
    }
}
void disableCSAD5204(int Pot)
{
    switch (Pot)
    {
    case 0:
        sleep_us(15);
        gpio_put(pin_nCS_DCP0, 1);
        sleep_us(15);
        break;
    case 1:
        sleep_us(15);
        gpio_put(pin_nCS_DCP1, 1);
        sleep_us(15);
        break;
    }
}

void setAllPots(int val)
{
    static int last_val = -1; // Tracks the last set value (-1 means uninitialized)
    uint8_t value = (uint8_t)val;
    uint8_t CMD[2] = {0};

    for (int pot = 0; pot < 2; pot++)
    {
        for (int chan = 0; chan < 4; chan++)
        {
            uint8_t channel = (uint8_t)(chan);
            CMD[0] = channel;
            CMD[1] = value;
            enableCSAD5204(pot);
            spi_write_blocking(SPI_PORT, CMD, 2);
            disableCSAD5204(pot);
        }
    }

    // Calculate delta and sleep time
    int delay_us = 2000;  // Default to max (2ms)
    if (last_val >= 0)
    {
        int delta = abs(val - last_val);
        delay_us = (int)((delta / 256.0f) * 2000.0f); // convert to microseconds
        if (delay_us > 2000) delay_us = 2000;
        if (delay_us < 0) delay_us = 0;
    }

    sleep_us(delay_us);
    last_val = val;
}
