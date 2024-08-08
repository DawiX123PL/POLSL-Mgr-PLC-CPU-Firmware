

#include <inttypes.h>
#include "main.h"
#include "spi.h"
#include "crc.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"

#include "plc_crc.hpp"
#include "plc_spi.hpp"

extern "C"
{

    constexpr int bytes_per_module = 32;

    struct GlobalMem
    {
        uint8_t input[bytes_per_module * max_io_modules];  // %Ix.x
        uint8_t output[bytes_per_module * max_io_modules]; // %Qx.x
        uint8_t memory[bytes_per_module * max_io_modules]; // %Mx.x
    };

    enum class DeviceID : uint8_t
    {
        UNNOWN = 0,
        ANALOG_INPUT = 1,
        DIGITAL_OUTPUT = 2,
    };

    struct IOModuleState
    {
        DeviceID device_id = DeviceID::UNNOWN;
        uint8_t status_byte;
        uint8_t error_byte;
        uint8_t digital_input = 0;
        uint8_t digital_output_level;
        uint8_t digital_output_enable;
        uint16_t analog_input0;
        uint16_t analog_input1;
        uint16_t analog_input2;
        uint16_t analog_input3;
        uint16_t analog_input4;
        uint16_t analog_input5;
        uint16_t analog_input6;
        uint16_t analog_input7;
        uint16_t supply_voltage;
    };

    struct IOModule
    {
        enum class Status
        {
            DISCONNECTED,
            CONNECTED,
        };

        Status status;
        IOModuleState module_state;
    };
}

// returns:
//  `true` if crc is correct
//  `false` on crc mismatch
bool ReadIOmodule(IOModule *module, int module_id)
{
    constexpr int frame_size = sizeof(IOModuleState) + 1; // last byte for crc
    uint8_t tx_data[frame_size] = {0};                    // do not send any command or data;
    uint8_t rx_data[frame_size] = {0};

    // calculate crc for transmited message
    uint8_t transmited_crc = crc_calculate(tx_data, frame_size - 1);
    tx_data[frame_size - 1] = transmited_crc;

    SelectModule(module_id);
    osDelay(1);
    SpiTransmitReceive(tx_data, rx_data, frame_size);
    osDelay(1);
    DeselectModule(module_id);

    // check for crc error
    if (crc_verify(rx_data, frame_size))
    {
        return false;
    }

    // store received data frame;
    module->module_state = *(IOModuleState *)rx_data;

    return true;
}

void ClearIOModuleError(IOModule *module, int module_id)
{
    // step 1 - send clear error cmd
}

void UpdateIOmodule(IOModule *module, int module_id)
{
}

void ReadAllIOmodules(IOModule *module, int count)
{
    for (int module_id = 0; module_id < max_io_modules; module_id++)
    {
        bool is_successful = false;

        for (int attempt = 0; attempt < 5; attempt++)
        {
            is_successful = ReadIOmodule(&module[module_id], module_id);
            if (is_successful)
            {
                break;
            }
        }

        if (is_successful)
        {
            module[module_id].status = IOModule::Status::CONNECTED;
        }
        else
        {
            module[module_id].status = IOModule::Status::DISCONNECTED;
        }
    }
}

void UpdateAllIOmodules(IOModule *module, int count)
{
    for (int module_id = 0; module_id < max_io_modules; module_id++)
    {
        UpdateIOmodule(&module[module_id], module_id);
    }
}

IOModule io_modules[max_io_modules] = {};

extern "C" void
IoModuleControlTaskFcn(void *argument)
{
    osDelay(10);
    DeselectAllModules();
    osDelay(10);

    ReadAllIOmodules(io_modules, max_io_modules);

    while (true)
    {
        // step 1 - update io modules
        ReadAllIOmodules(io_modules, max_io_modules);
        // step 2 - execute user code
    }
}
