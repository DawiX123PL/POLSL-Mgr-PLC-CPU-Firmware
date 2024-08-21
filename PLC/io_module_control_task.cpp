

#include <inttypes.h>
#include <array>
#include "main.h"
#include "spi.h"
#include "crc.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#include "plc_crc.hpp"
#include "plc_spi.hpp"

#include "plc_module.hpp"
#include "plc_user_code.hpp"
#include "plc_status.hpp"

// protected with program_memory_write_mutHandle
CodeBlock program_memory;

// User Code memory;
std::array<uint8_t, 90> program_local_memory;

// Global Program memory
GlobalMem global_program_memory;

// modules 
IOModule io_modules[max_io_modules] = {};

extern osEventFlagsId_t plc_status_flagHandle;
PlcStatus plc_status = PlcStatus::STOP;

void HandleStartStopRequest()
{
    uint32_t expected_flags = PlcRequestFlags::RUN | PlcRequestFlags::STOP;
    uint32_t flag = osEventFlagsWait(plc_status_flagHandle, expected_flags, osFlagsWaitAny, 0);

    if (flag & osFlagsError)
    {
        return;
    }

    if ((flag & PlcRequestFlags::RUN) && (plc_status != PlcStatus::RUN))
    {
        // begin startup sequence
        plc_status = PlcStatus::STARTING;
    }

    if (flag & PlcRequestFlags::STOP && (plc_status != PlcStatus::STOP))
    {
        // begin stop sequence
        plc_status = PlcStatus::STOPING;
    }
}

void StopSequence()
{
    osMutexRelease(program_memory_write_mutHandle);

    plc_status = PlcStatus::STOP;
}

void StartSequence()
{
    osStatus_t os_status = osMutexAcquire(program_memory_write_mutHandle, 0);

    if (os_status != osOK)
    {
        plc_status = PlcStatus::STOPING;
        return;
    }

    if (!program_memory.Verify())
    {
        plc_status = PlcStatus::STOPING;
        return;
    }

    plc_status = PlcStatus::RUN;
}

void RunProgram(bool first_scan)
{
    // get all functions from symbol table

    SymbolTable *st = &program_memory.code.symbol_table;

    ProgramPtr main_prog = RelativeToGlobalPointer(st, st->main_program_relptr);
    ProgramInitPtr main_prog_init = RelativeToGlobalPointer(st, st->main_program_init_relptr);
    uint32_t main_prog_size = st->program_size;

    // check if program has enough memory available
    if (program_local_memory.size() < main_prog_size)
    {
        plc_status = PlcStatus::STOPING;
        return;
    }

    // execute user code
    if(first_scan)
    {
        program_local_memory.fill(0xAA);
        main_prog_init(&global_program_memory, program_local_memory.data());
    }

    main_prog(&global_program_memory, program_local_memory.data());
    
}

extern "C" void
IoModuleControlTaskFcn(void *argument)
{

    osDelay(10);

    for (int i = 0; i < max_io_modules; i++)
    {
        SpiDeselect(i);
    }

    osDelay(100); //wait for modules to startup

   	IOmoduleClearErrorAll(io_modules,  max_io_modules);

    osDelay(10);

    IOmoduleReadAll(io_modules, max_io_modules);

    bool first_scan = true;

    while (true)
    {
        IOmoduleUpdateAll(io_modules, max_io_modules);

        if (plc_status == PlcStatus::STARTING)
        {
            StartSequence();
            first_scan = true;
        }

        if (plc_status == PlcStatus::STOPING)
        {
            StopSequence();
            first_scan = true;
        }

        if (plc_status == PlcStatus::RUN)
        {
            RunProgram(first_scan);
            first_scan = false;
        }

        // get requests to start or stop
        HandleStartStopRequest();
    }
}
