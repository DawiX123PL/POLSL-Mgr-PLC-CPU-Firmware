#pragma once

#include <inttypes.h>

// maximum amout of io modules connected to PLC
constexpr int max_io_modules = 5;


extern "C"
{

    constexpr int bytes_per_module = 32;

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

bool IOmoduleRead(IOModule *module, int module_id);
void IOmoduleWrite(IOModule *module, int module_id);
void IOModuleClearError(IOModule *module, int module_id);
bool IOmoduleUpdate(IOModule *module, int module_id);
void IOmoduleReadAll(IOModule *module, int count);
void IOmoduleUpdateAll(IOModule *module, int count);
void IOmoduleClearErrorAll(IOModule *module, int count);
