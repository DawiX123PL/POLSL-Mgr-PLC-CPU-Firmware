#pragma once

#include <inttypes.h>
#include "stm32f4xx.h"
#include "crc.h"

// Calculate CRC
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint8_t crc_calculate(uint8_t *buffer, uint32_t size);

// verifies CRC of dataframe.
// Function expect last byte to be crc.
// returns:
//  - true if crc is correct
//  - false on crc mismatch
bool crc_verify(uint8_t *buffer, uint32_t size);