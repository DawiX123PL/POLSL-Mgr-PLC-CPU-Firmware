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
    if (Crc8Verify(rx_data, frame_size))
    {
        return false;
    }

    // store received data frame;
    module->module_state = *(IOModuleState *)rx_data;

    return true;
}

void IOModuleClearError(IOModule *module, int module_id)
{
    // step 1 - send clear error cmd
}

void IOmoduleUpdate(IOModule *module, int module_id)
{
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
        IOmoduleUpdate(&module[module_id], module_id);
    }
}