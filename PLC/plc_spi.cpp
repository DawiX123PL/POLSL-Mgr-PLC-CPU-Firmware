#include "plc_spi.hpp"

// return CS port for specified module number
GPIO_TypeDef *ModulePort(int module_id)
{
    switch (module_id)
    {
    case 0:
        return MODULE_CS0_GPIO_Port;
    case 1:
        return MODULE_CS1_GPIO_Port;
    case 2:
        return MODULE_CS2_GPIO_Port;
    case 3:
        return MODULE_CS3_GPIO_Port;
    case 4:
        return MODULE_CS4_GPIO_Port;

    default:
        // todo ; handle error case
        return MODULE_CS4_GPIO_Port;
        break;
    }
}

// return CS pin for specified module number
uint16_t ModulePin(int module_id)
{
    switch (module_id)
    {
    case 0:
        return MODULE_CS0_Pin;
    case 1:
        return MODULE_CS1_Pin;
    case 2:
        return MODULE_CS2_Pin;
    case 3:
        return MODULE_CS3_Pin;
    case 4:
        return MODULE_CS4_Pin;

    default:
        // todo ; handle error case
        return MODULE_CS4_Pin;
        break;
    }
}

// 1 (true) - module selected
// 0 (false) - module not selected
void SelectModuleEx(int module_id, bool select)
{
    GPIO_PinState pin_state = select ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(ModulePort(module_id), ModulePin(module_id), pin_state);
}

void SelectModule(int module_id)
{
    SelectModuleEx(module_id, true);
}

void DeselectModule(int module_id)
{
    SelectModuleEx(module_id, false);
}

// deselets all io modules
void DeselectAllModules()
{
    for (int i = 0; i < max_io_modules; i++)
    {
        SelectModuleEx(i, false);
    }
}

// transmit and receive data from io module
void SpiTransmitReceive(uint8_t *tx_data, uint8_t *rx_data, int frame_size)
{
    HAL_SPI_TransmitReceive(&hspi_module, tx_data, rx_data, frame_size, HAL_MAX_DELAY);
}
