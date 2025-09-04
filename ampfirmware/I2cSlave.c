#include "PinDeff.h"
#include "I2cSlave.h"
#include "Calibration.h"


#define I2C_TIMEOUT_MS 5  // Timeout period in milliseconds

void i2c_slave_polling() {
    absolute_time_t start_time = get_absolute_time();

    while (true) {
        uint32_t status = i2c_get_hw(I2C_PORT)->intr_stat;

        // Timeout check
        if (absolute_time_diff_us(start_time, get_absolute_time()) > I2C_TIMEOUT_MS * 1000) {
            rx_index = 0;  // Reset RX buffer to avoid partial data handling
            return;
        }

        // Handle receive data from master
        if (status & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
            while (status & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
                uint8_t data = (uint8_t)(i2c_get_hw(I2C_PORT)->data_cmd & 0xFF);

                if (rx_index < sizeof(rx_buffer)) {
                    rx_buffer[rx_index++] = data;
                } else {
                    rx_index = 0; // Overflow
                }

                status = i2c_get_hw(I2C_PORT)->intr_stat;
                start_time = get_absolute_time();  // Reset timeout since activity occurred
            }

            switch (rx_buffer[0]) {
                case 0xAA: AmpON = true; break;
                case 0xAB: AmpOFF = true; break;
                case 0xAC: CalibrateAmps = true; break;
                case 0xAD: AdcTurnOn = true; break;
                case 0xAE: AdcShutDown = true; break;
                case 0xAF: resetADC = true; break;
                case 0xBA: startSample = false; break;
                case 0xBB: startSample = true; break;
                case 0xBC: StartSystemDraw = true; break;
                case 0xBD: StopSystemDraw = true; break;
                case 0xBE: testMode = true; break;
                case 0xBF: {
                    uint16_t target = (((uint16_t)rx_buffer[1] << 8) | rx_buffer[2]);
                    biasTarget = target;
                    CalibrateAmps = true;
                    rx_index = 0;
                    break;
                }
                default:
                    rx_index = 0;
                    break;
            }
            return;
        }

        // Handle master requesting data
        if (status & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {
            static uint8_t tx_index = 0;
            if (tx_index < sizeof(tx_buffer)) {
                i2c_get_hw(I2C_PORT)->data_cmd = tx_buffer[tx_index++];
            } else {
                i2c_get_hw(I2C_PORT)->data_cmd = 0x00;
            }
            i2c_get_hw(I2C_PORT)->clr_rd_req;
            if (tx_index >= sizeof(tx_buffer)) {
                tx_index = 0;
            }
            return;
        }

        // Handle stop condition
        if (status & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
            i2c_get_hw(I2C_PORT)->clr_stop_det;
            rx_index = 0;
            return;
        }

        // Yield CPU briefly to avoid busy-waiting
        tight_loop_contents();
    }
}

void initI2Cslave()
{
    uint8_t I2Caddress = findAdress();

    gpio_init(pin_SDA);
    gpio_set_function(pin_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(pin_SDA);

    gpio_init(pin_SCL);
    gpio_set_function(pin_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(pin_SCL);

    // Initialize I2C in slave mode
    i2c_init(I2C_PORT, 400000); // 400 kHz baudrate
    i2c_set_slave_mode(I2C_PORT, true, I2Caddress);
}

uint8_t findAdress()
{
    bool addressPin0 = gpio_get(pin_i2c_id0);
    bool addressPin1 = gpio_get(pin_i2c_id1);
    uint8_t addressPinout = 0x0;
    addressPinout |= ((uint8_t)addressPin1 << 1);
    addressPinout |= ((uint8_t)addressPin0 << 0);
    ampPos = addressPinout;
    switch (addressPinout)
    {
    case 0x0:
        return I2C_address0;
    case 0x1:
        return I2C_address1;
    case 0x2:
        return I2C_address2;
    case 0x3:
        return I2C_address3;
    }
    return I2C_address0; // Default return
}

