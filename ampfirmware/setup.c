#include "PinDeff.h"
#include "MCP3461.h"
#include "I2cSlave.h"
#include "Calibration.h"
static uint32_t DebouceTime = 1000;
volatile uint32_t lastInterupt = 0;

void IRQTrig(uint gpio, uint32_t events) {
    if(gpio == pin_nIRQ){
        __atomic_store_n(&IRQFLAG, true, __ATOMIC_RELAXED); // Atomic store for IRQFLAG
    }
    else if(gpio == ForceCalPin){
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        
        if(currentTime - lastInterupt > DebouceTime){
            mutex_enter_blocking(&my_mutex);
            CalibrateAmps = true;
            mutex_exit(&my_mutex);
            lastInterupt = currentTime;
        }
     
    }
}

void setup(){

    xosc_hw->startup = 0x3fff;
     // Allow time for the XOSC to stabilize
    
    stdio_init_all();
    sleep_ms(100); // amps need time to settel before proper reading can be done
    mutex_init(&my_mutex);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(pin_nCS_adc);
    gpio_set_dir(pin_nCS_adc,  GPIO_OUT);
    gpio_put(pin_nCS_adc,  1);

    gpio_init(pin_nCS_DCP0);
    gpio_set_dir(pin_nCS_DCP0, GPIO_OUT);
    gpio_put(pin_nCS_DCP0, 1);

    gpio_init(pin_nCS_DCP1);
    gpio_set_dir(pin_nCS_DCP1, GPIO_OUT);
    gpio_put(pin_nCS_DCP1, 1);

    gpio_init(pin_mux0_a0);
    gpio_set_dir(pin_mux0_a0, GPIO_OUT);
    gpio_put(pin_mux0_a0, 0);
    
    gpio_init(pin_mux0_a1);
    gpio_set_dir(pin_mux0_a1, GPIO_OUT);
    gpio_put(pin_mux0_a1, 0);

    gpio_init(pin_mux1_a0);
    gpio_set_dir(pin_mux1_a0, GPIO_OUT);
    gpio_put(pin_mux1_a0, 0);

    gpio_init(pin_mux1_a1);
    gpio_set_dir(pin_mux1_a1, GPIO_OUT);
    gpio_put(pin_mux1_a1, 0);

    gpio_init(pin_nPD);
    gpio_set_dir(pin_nPD, GPIO_OUT);
    gpio_put(pin_nPD, 0);



    

    gpio_init(pin_nPD_LED);
    gpio_set_dir(pin_nPD_LED, GPIO_OUT);
    gpio_put(pin_nPD_LED, 1);

    
    

    gpio_init(pin_nIRQ);
    gpio_set_dir(pin_nIRQ, GPIO_IN);
    

    gpio_init(ForceCalPin);
    gpio_set_dir(ForceCalPin, GPIO_IN);
    gpio_pull_up(ForceCalPin);
    multicore_fifo_clear_irq();
    gpio_set_irq_enabled_with_callback(ForceCalPin, GPIO_IRQ_EDGE_FALL, true, &IRQTrig);
    gpio_set_irq_enabled(pin_nIRQ, GPIO_IRQ_EDGE_FALL, true);


    // SPI initialisation. This example will use SPI at 20MHz.
    uint16_t biasVoltage = read_value_from_flash();

    // Validate flash value
    if (biasVoltage == 0xFFFF || biasVoltage < 8000 || biasVoltage > 12000) {
        biasVoltage = 12288;  // Use a known-safe default
    }

    biasTarget = biasVoltage;
    spi_init(SPI_PORT, 20*1000000);
    gpio_set_function(pin_spiRX, GPIO_FUNC_SPI);
    gpio_set_function(pin_spiSCK,  GPIO_FUNC_SPI);
    gpio_set_function(pin_spiTX, GPIO_FUNC_SPI);
    multicore_launch_core1(core1_entry);
    sleep_us(100);

}

void setupCore1() {


    adc_init();
    adc_gpio_init(pin_adc0);
    adc_gpio_init(pin_adc1);
    adc_gpio_init(pin_adc2);
    adc_gpio_init(pin_adc3);

    gpio_init(pin_shtdwn_LED);
    gpio_set_dir(pin_shtdwn_LED, GPIO_OUT);
    gpio_put(pin_shtdwn_LED, 0);

    gpio_init(pin_i2c_id0);
    gpio_set_dir(pin_i2c_id0, GPIO_IN);

    gpio_init(pin_i2c_id1);
    gpio_set_dir(pin_i2c_id1, GPIO_IN);

    gpio_init(TestCRQpin);
    gpio_set_dir(TestCRQpin, GPIO_OUT);
    gpio_put(TestCRQpin, 1);

    

    initI2Cslave();
    sleep_us(100);
}
