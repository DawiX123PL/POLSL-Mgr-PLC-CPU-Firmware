#pragma once

#include <inttypes.h>
#include "stm32f4xx.h"
#include "crc.h"

// Calculate CRC using hardware modules
// # NOT THREAD SAFE
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint8_t Crc8CalculateHard(uint8_t *buffer, uint32_t size);

// Calculate CRC using hardware modules
// # NOT THREAD SAFE
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint32_t Crc32CalculateHard(uint8_t *buffer, uint32_t size);

// Calculate CRC using software only
// # (Thread safe)
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint8_t Crc8CalculateSoft(uint8_t *data, uint32_t size);

// Calculate CRC using software only
// # Thread safe
//
// Polynomial       - 0x4C11DB7
// Initial value    - 0xFFFFFFFF
// Final XOR        - 0x0
// Input Reflected  - NOPE
// Output Reflected - NOPE
uint32_t Crc32CalculateSoft(uint8_t *data, uint32_t size);

// # NOT THREAD SAFE
// verifies CRC of dataframe.
// Function expect last byte to be crc.
// returns:
//  - true if crc is correct
//  - false on crc mismatch
bool Crc8Verify(uint8_t *buffer, uint32_t size);