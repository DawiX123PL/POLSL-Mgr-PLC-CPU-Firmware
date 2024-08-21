#include "plc_module.hpp"

#include "plc_spi.hpp"
#include "plc_crc.hpp"
#include "cmsis_os.h"

// returns:
//  `true` if crc is correct
//  `false` on crc mismatch
bool IOmoduleRead(IOModule *module, int module_id)
{
    constexpr int frame_size = sizeof(IOModuleState) + 1; // last byte for crc
    uint8_t tx_data[frame_size] = {0};                    // do not send any command or data;
    uint8_t rx_data[frame_size] = {0};

    // calculate crc for transmited message
    uint8_t transmited_crc = Crc8CalculateHard(tx_data, frame_size - 1);
    tx_data[frame_size - 1] = transmited_crc;

    SpiSelect(module_id);
    osDelay(1);
    SpiTransmitReceive(tx_data, rx_data, frame_size);
    osDelay(1);
    SpiDeselect(module_id);

    // check for crc error

    if (!Crc8Verify(rx_data, frame_size))
    {
        return false;
    }

    // store received data frame;
    module->module_state = *(IOModuleState *)rx_data;

    return true;
}

void IOmoduleWrite(IOModule *module, int module_id)
{
    constexpr uint8_t command_write_dq = 2;
    constexpr int frame_size = 4; // [command_byte, digital_output_level, digital_output_enable, crc];
    uint8_t tx_data[frame_size] =
        {
            command_write_dq,
            module->module_state.digital_output_level,
            module->module_state.digital_output_enable,
            0};

    uint8_t rx_data[frame_size] = {0};

    // calculate crc
    tx_data[frame_size - 1] = Crc8CalculateHard(tx_data, frame_size - 1);

    SpiSelect(module_id);
    osDelay(1);
    SpiTransmitReceive(tx_data, rx_data, frame_size);
    osDelay(1);
    SpiDeselect(module_id);

    // ignore incomming bytes
}

void IOModuleClearError(IOModule *module, int module_id)
{
    constexpr uint8_t command_clear_error = 1;
    constexpr int frame_size = 2; // [command_byte, crc];
    uint8_t tx_data[frame_size] =
        {
            command_clear_error,
            0};

    uint8_t rx_data[frame_size] = {0};

    // calculate crc
//    tx_data[frame_size - 1] = Crc8CalculateHard(tx_data, frame_size - 1);
    tx_data[frame_size - 1] = Crc8CalculateSoft(tx_data, frame_size - 1);

    SpiSelect(module_id);
    osDelay(1);
    SpiTransmitReceive(tx_data, rx_data, frame_size);
    osDelay(1);
    SpiDeselect(module_id);

    // ignore incomming bytes
}

bool IomoduleClearErrorAndRead(IOModule *module, int module_id)
{
    IOModuleClearError(module, module_id);
    osDelay(1);
    bool is_successful = IOmoduleRead(module, module_id);
    if (!is_successful)
    {
        return false;
    }

    return true;
}

bool IOmoduleUpdate(IOModule *module, int module_id)
{

    uint8_t digital_output_level = module->module_state.digital_output_level;
    uint8_t digital_output_enable = module->module_state.digital_output_enable;

    IOmoduleWrite(module, module_id);
    osDelay(1);
    bool is_successful = IOmoduleRead(module, module_id);
    if (!is_successful)
    {
        return false;
    }

    return true;

    // check if data was succesfuly written

    // if (digital_output_level == module->module_state.digital_output_level &&
    //     digital_output_enable == module->module_state.digital_output_enable)
    // {
    //     is_successful = true;
    //     return true;
    // }
    // else
    // {
    //     return false;
    // }
}

void IOmoduleReadAll(IOModule *module, int count)
{
    for (int module_id = 0; module_id < max_io_modules; module_id++)
    {
        bool is_successful = false;

        for (int attempt = 0; attempt < 5; attempt++)
        {
            is_successful = IOmoduleRead(&module[module_id], module_id);
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

void IOmoduleUpdateAll(IOModule *module, int count)
{
    for (int module_id = 0; module_id < max_io_modules; module_id++)
    {
        bool is_successful = false;

        for (int attempt = 0; attempt < 5; attempt++)
        {
            is_successful = IOmoduleUpdate(&module[module_id], module_id);
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

void IOmoduleClearErrorAll(IOModule *module, int count)
{
    for (int module_id = 0; module_id < max_io_modules; module_id++)
    {
        bool is_successful = false;

        for (int attempt = 0; attempt < 5; attempt++)
        {
            is_successful = IomoduleClearErrorAndRead(&module[module_id], module_id);
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
