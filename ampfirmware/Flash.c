#include "PinDeff.h"

static const uint32_t FLASH_PAGE = 256;
static const uint32_t FLASH_SECTOR = 4096;
static const uint32_t FLASH_OFFSET = FLASH_TARGET_OFFSET;
static const uint8_t *flash_ptr = (const uint8_t *)(XIP_BASE + FLASH_OFFSET);

// Helper: called safely to erase sector
static void call_flash_erase(void *param) {
    flash_range_erase(FLASH_OFFSET, FLASH_SECTOR);
}

// Helper: called safely to program page
static void call_flash_program(void *param) {
    uintptr_t *params = (uintptr_t *)param;
    uint32_t offset = params[0];
    const uint8_t *data = (const uint8_t *)params[1];
    flash_range_program(offset, data, FLASH_PAGE);
}

uint16_t read_value_from_flash() {
    uint16_t value;
    memcpy(&value, flash_ptr, sizeof(value));
    return value;
}

void __not_in_flash_func(save_value_to_flash)(uint16_t value) {
    uint8_t buffer[256] = {0};  // FLASH_PAGE = 256 bytes fixed
    memcpy(buffer, &value, sizeof(value));

    // Disable interrupts
    uint32_t ints = save_and_disable_interrupts();

    // Erase safely
    int rc = flash_safe_execute(call_flash_erase, NULL, UINT32_MAX);
    hard_assert(rc == PICO_OK);

    // Program safely
    uintptr_t params[] = { FLASH_OFFSET, (uintptr_t)buffer };
    rc = flash_safe_execute(call_flash_program, params, UINT32_MAX);
    hard_assert(rc == PICO_OK);

    // Restore interrupts
    restore_interrupts(ints);
}
