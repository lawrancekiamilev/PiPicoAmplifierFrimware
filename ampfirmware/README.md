# CDS Amplifier Controller Firmware

## Overview
This firmware is designed to run on a Raspberry Pi Pico (or compatible RP2040 board) and manages an amplifier system through I2C commands, analog signal acquisition (via the MCP3461 ADC), and digital potentiometer configuration (via AD5204). It supports multiple amplifier slots, system calibration, temperature monitoring, EEPROM storage, and safe multi-core operation.

---

## 🧱 Core Components

### `CdsAmpController.c`
- Main system logic
- Manages inter-core operations and state machines
- Coordinates ADC reads, I2C commands, calibration triggers

### `AD5204.c/h`
- Interfaces with AD5204 digital potentiometers
- Functions:
  - `SetPot()`, `setAllPots()`
  - `enableCSAD5204()` / `disableCSAD5204()`
  - Implements adaptive delay based on value delta

### `Calibration.c/h`
- Calibration and characterization routines
- Includes:
  - Slope/offset measurements
  - EEPROM storage trigger
  - DAC ramping, bias pulse generation

### `I2cSlave.c/h`
- Implements I2C slave in polling mode
- Command decoding:
  - `0xAA` = AmpON, `0xAB` = AmpOFF, `0xAC` = Calibrate, etc.
- Sends 27-byte status buffer on master read

### `MCP3461.c/h` & `MCP3461Config.c`
- Configures and reads the MCP3461 ADC
- Includes:
  - VCM checks
  - Single-shot conversion with verification
  - `getADCtemp()` for internal temperature sensor

### `Flash.c`
- Non-volatile storage via RP2040 flash
- Stores 16-bit `biasTarget` value with 256-byte aligned buffer

### `GlobalVars.c`
- Centralized shared variables:
  - Flags for amp/ADC control, test mode, calibration
  - TX/RX buffers
  - Temperature and mutex locks

### `Mux.c`
- Controls multiplexer channel pins (`pin_mux0_a0/a1`, `pin_mux1_a0/a1`)

### `PinDeff.h`
- Macro definitions for all GPIO, SPI, I2C pinouts used in the project

---

## 🔌 I2C Communication Protocol

- **I2C Slave** (up to 4 addresses supported via ID pins)
- **Commands**:  
  - `0xAA` → Amp ON  
  - `0xAB` → Amp OFF  
  - `0xAC` → Calibrate Amps  
  - `0xAD` → ADC ON  
  - `0xAE` → ADC OFF  
  - `0xAF` → ADC Reset  
  - `0xBA` → Stop Sampling  
  - `0xBB` → Start Sampling  
  - `0xBC` → Begin System Draw  
  - `0xBD` → Stop System Draw  
  - `0xBE` → Enable Test Mode  
  - `0xBF <high><low>` → Set Bias Target & Calibrate

- **Responses**: 27-byte buffer with system state and measurement data

---

## 🧠 Multi-Core Architecture

| Core 0                | Core 1                         |
|-----------------------|--------------------------------|
| Main control loop     | Real-time polling + I2C RX/TX |
| ADC + Amp state mgmt  | Temperature + status update   |

- Uses `mutex_t my_mutex` for flag access control
- `i2c_slave_polling()` runs continuously on core1

---

## 🧮 Status Byte Encoding (`tx_buffer[0]`)

| Bit | Meaning           |
|-----|-------------------|
| 7-6 | `ampPos` (2 bits) |
| 5   | AmpPwrStatus      |
| 4   | ADCPwrStatus      |
| 3   | VCMStatus         |
| 2   | startSample       |
| 1   | OffsetStatus      |
| 0   | TempStatus        |

---

## 🧾 Flash Memory

- Persistent calibration storage:
  - `save_value_to_flash(uint16_t)`
  - `read_value_from_flash()`

---

## 🛠 Build Requirements

- **Platform**: Raspberry Pi Pico SDK (CMake)
- **Compiler**: ARM GCC toolchain
- **Dependencies**:
  - Multicore
  - Mutex
  - SPI/I2C/Flash/GPIO drivers

---

## 📌 Notes

- Uses polling instead of IRQs for I2C for better control
- All ADC reads include retry logic for reliability
- Designed to interface with external host via 4-byte commands
