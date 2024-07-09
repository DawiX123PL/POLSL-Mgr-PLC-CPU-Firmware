#pragma once

#include <inttypes.h>
#include "stm32f4xx.h"
#include "spi.h"

// maximum amout of io modules connected to PLC
constexpr int max_io_modules = 5;

// return CS port for specified module number
GPIO_TypeDef *ModulePort(int module_id);

// return CS pin for specified module number
uint16_t ModulePin(int module_id);

// 1 (true) - module selected
// 0 (false) - module not selected
void SelectModuleEx(int module_id, bool level);

void SelectModule(int module_id);

void DeselectModule(int module_id);

// deselets all io modules
void DeselectAllModules();

// transmit and receive data from io module
void SpiTransmitReceive(uint8_t *tx_data, uint8_t *rx_data, int frame_size);
