#pragma once

#include <inttypes.h>
#include "stm32f4xx.h"
#include "spi.h"

// // maximum amout of io modules connected to PLC
// constexpr int max_io_modules = 5;


void SpiSelect(int module_id);

void SpiDeselect(int module_id);

// deselets all io modules
void SpiDeselectAll();

// transmit and receive data from io module
void SpiTransmitReceive(uint8_t *tx_data, uint8_t *rx_data, int frame_size);
