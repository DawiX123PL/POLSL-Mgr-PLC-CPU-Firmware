

#include <inttypes.h>
#include <array>
#include "main.h"
#include "spi.h"
#include "crc.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"

#include "plc_crc.hpp"
#include "plc_spi.hpp"

#include "plc_module.hpp"
#include "plc_user_code.hpp"


std::array<uint8_t, sizeof(CodeBlock)> program_memory;


IOModule io_modules[max_io_modules] = {};

extern "C" void
IoModuleControlTaskFcn(void *argument)
{

    while(1){
    osDelay(10);

    }

    for (int i = 0; i < max_io_modules; i++)
    {
        SpiDeselect(i);
    }

    osDelay(10);

    IOmoduleReadAll(io_modules, max_io_modules);

    while (true)
    {
        // step 1 - update io modules
        IOmoduleReadAll(io_modules, max_io_modules);
        // step 2 - execute user code
    }
}
