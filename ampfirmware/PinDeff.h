#ifndef PinDeff_h
#define PinDeff_h

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/mutex.h"
#include "pico/multicore.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/bootrom.h"
#include "pico/flash.h"
#include "hardware/watchdog.h"
#include "hardware/xosc.h"


#define SPI_PORT                    spi0
#define pin_spiRX                   16      // MISO SPI pin
#define pin_spiSCK                  18      // SCK SPI pin
#define pin_spiTX                   19      // MOSI SPI pin


#define pin_nCS_adc                 17      // chip select adc
#define pin_nCS_DCP0                20      // chip select dcp0
#define pin_nCS_DCP1                21      // chip select dcp1

#define pin_nIRQ                    4       // !IRQ pin for adc


#define pin_mux0_a0                 10      // a0 select pin for mux0
#define pin_mux0_a1                 9       // a1 select pin for mux0
#define pin_mux1_a0                 8       // a0 select pin for mux1
#define pin_mux1_a1                 7       // a1 select pin for mux1

#define pin_adc0                    26      // on die adc0 for rp2040
#define pin_adc1                    27      // on die adc1 for rp2040
#define pin_adc2                    28      // on die adc2 for rp2040
#define pin_adc3                    29      // on die adc3 for rp2040

#define pin_nPD_LED                 24      // LED pin for power down indicator
#define pin_bias_LED                2       // LED pin for bias indicator
#define pin_shtdwn_LED              22      // LED pin for thermal shutdownutdown down indicator
#define pin_nPD                     11      // power down pin for op-amps

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - 4096)

#define TestCRQpin                  12
#define ForceCalPin                 25

extern mutex_t my_mutex;

extern const float vref;

extern volatile float ADCtemp;

extern volatile float Temps[4];
extern volatile uint8_t IntTemps[8];
extern volatile bool Savebias;
extern volatile uint16_t biasToSave;

void IRQTrig(uint gpio, uint32_t events);
void IterateMux(int chan);
void setup();
void setupCore1();
void core1_entry();

void checkSystemTemp();
void CheckChanVoltages();
void populateTempBuffer();
void save_value_to_flash(uint16_t value);
uint16_t read_value_from_flash();
#endif