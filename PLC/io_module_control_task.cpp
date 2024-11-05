

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
#include "performance.hpp"

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
    else if (flag & PlcRequestFlags::STOP && (plc_status != PlcStatus::STOP))
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
    if (first_scan)
    {
        program_local_memory.fill(0xAA);
        main_prog_init(&global_program_memory, program_local_memory.data());
    }

    main_prog(&global_program_memory, program_local_memory.data());
}

void CopyInputToGlobalMemory()
{
    for (int mod_nr = 0; mod_nr < max_io_modules; mod_nr++)
    {
        uint16_t *ptr = (uint16_t *)global_program_memory.input[mod_nr];
        *ptr++ = io_modules[mod_nr].module_state.digital_input;
        *ptr++ = io_modules[mod_nr].module_state.analog_input0;
        *ptr++ = io_modules[mod_nr].module_state.analog_input1;
        *ptr++ = io_modules[mod_nr].module_state.analog_input2;
        *ptr++ = io_modules[mod_nr].module_state.analog_input3;
        *ptr++ = io_modules[mod_nr].module_state.analog_input4;
        *ptr++ = io_modules[mod_nr].module_state.analog_input5;
        *ptr++ = io_modules[mod_nr].module_state.analog_input6;
        *ptr++ = io_modules[mod_nr].module_state.analog_input7;
        *ptr++ = io_modules[mod_nr].module_state.supply_voltage;

        assert_param((ptr - (uint16_t *)global_program_memory.input[mod_nr]) < bytes_per_module);
    }
}

void CopyOutputFromGlobalMemory()
{
    for (int mod_nr = 0; mod_nr < max_io_modules; mod_nr++)
    {
        uint8_t *ptr = (uint8_t *)global_program_memory.output[mod_nr];
        io_modules[mod_nr].module_state.digital_output_level = *ptr++;
        io_modules[mod_nr].module_state.digital_output_enable = *ptr++;

        assert_param((ptr - (uint8_t *)global_program_memory.output[mod_nr]) < bytes_per_module);
    }
}

extern "C" void
IoModuleControlTaskFcn(void *argument)
{

    osDelay(10);

    for (int i = 0; i < max_io_modules; i++)
    {
        SpiDeselect(i);
    }

    osDelay(100); // wait for modules to startup

    for (int i = 0; i < 10; i++)
    {
        IOmoduleClearErrorAll(io_modules, max_io_modules);
    }

    osDelay(10);

    IOmoduleReadAll(io_modules, max_io_modules);

    bool first_scan = true;

    Performance::InitTimers();
    Performance::StartTimer();

    while (true)
    {

        IOmoduleUpdateAll(io_modules, max_io_modules);

        Performance::module_update_time.Update();

        if (plc_status == PlcStatus::STARTING)
        {
            StartSequence();
            first_scan = true;
            Performance::start_sequence_time.Update();
        }

        if (plc_status == PlcStatus::STOPING)
        {
            StopSequence();
            first_scan = true;
            Performance::stop_sequence_time.Update();
        }

        if (plc_status == PlcStatus::RUN)
        {
            CopyInputToGlobalMemory();
            RunProgram(first_scan);
            CopyOutputFromGlobalMemory();

            if (first_scan)
            {
                Performance::program_first_scan_time.Update();
            }
            else
            {
                Performance::program_execution_time.Update();
            }

            first_scan = false;
        }

        //

        // get requests to start or stop
        HandleStartStopRequest();

        Performance::requests_time.Update();
    }
}
