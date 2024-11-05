
#include "tcp_command_handlers.hpp"
#include <array>
#include <algorithm>
#include "plc_user_code.hpp"
#include "plc_crc.hpp"
#include "plc_status.hpp"
#include "performance.hpp"
#include <float.h>

extern osEventFlagsId_t plc_status_flagHandle;

bool IsFlagError(uint32_t flag)
{
    return (flag & osFlagsError) != 0;
}

namespace TcpCommandHandle
{
    void UnnownCommand(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        tx_data_frame.Push("ERROR");
        tx_data_frame.Push("Unnown command");
    }

    void Ping(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        tx_data_frame.Push("OK");
    }

    void Start(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        uint32_t flag = osEventFlagsSet(plc_status_flagHandle, PlcRequestFlags::RUN);
        // check for error
        if (IsFlagError(flag))
        {
            tx_data_frame.Push("ERROR");
        }
        else
        {
            tx_data_frame.Push("OK");
        }
    }

    void Stop(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        uint32_t flag = osEventFlagsSet(plc_status_flagHandle, PlcRequestFlags::STOP);
        // check for error
        if (IsFlagError(flag))
        {
            tx_data_frame.Push("ERROR");
        }
        else
        {
            tx_data_frame.Push("OK");
        }
    }

    void Status(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        // if flag is not 0 that means plc is still processing request
        uint32_t flag = osEventFlagsGet(plc_status_flagHandle);
        if (flag)
        {
            tx_data_frame.Push("WAIT");
            return;
        }
    }

    void ProgMem(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        std::string_view operation_type = rx_data_frame[1].Get<std::string_view>();
        if (operation_type == "R") // read
        {
            ProgMemRead(rx_data_frame, tx_data_frame);
        }
        if (operation_type == "W") // write
        {
            ProgMemWrite(rx_data_frame, tx_data_frame);
        }
        if (operation_type == "C") // clear
        {
            ProgMemClear(rx_data_frame, tx_data_frame);
        }
        if (operation_type == "VERIFY")
        {
            ProgMemVerify(rx_data_frame, tx_data_frame);
        }
    }

    void ProgMemVerify(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        uint32_t code_crc;
        uint32_t calculated_crc;
        if (program_memory.Verify(&code_crc, &calculated_crc))
        {
            tx_data_frame.Push("OK");
            return;
        }
        else
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("CRC Mismatch");
            tx_data_frame.Push(code_crc);
            tx_data_frame.Push(calculated_crc);
            return;
        }
    }

    void ProgMemWrite(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        uint32_t address = 0;
        bool is_address_ok = rx_data_frame[2].GetIfExist(&address);

        if (!is_address_ok)
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Invalid Address");
            return;
        }

        if (address >= program_memory.size())
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Outside of available memory");
            return;
        }

        osStatus_t os_status = osMutexAcquire(program_memory_write_mutHandle, 0);

        if (os_status != osOK)
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Cannot Access PROGMEM");
            return;
        }

        uint8_t *mem_ptr = program_memory.begin() + address;
        uint8_t *mem_end = program_memory.end();
        uint32_t mem_count = mem_end - mem_ptr;

        uint32_t readed_bytes;
        bool is_hex_ok = rx_data_frame[3].GetHex(mem_ptr, mem_count, &readed_bytes);

        os_status = osMutexRelease(program_memory_write_mutHandle);

        if (!is_hex_ok)
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Invalid Hex");
            return;
        }

        tx_data_frame.Push("OK");
        return;
    }

    void ProgMemRead(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        uint32_t address = 0;
        bool is_address_ok = rx_data_frame[2].GetIfExist(&address);

        if (!is_address_ok)
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Invalid Address");
            return;
        }

        uint32_t size = 0;
        bool is_size_ok = rx_data_frame[3].GetIfExist(&size);

        if (!is_size_ok)
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Invalid Number of bytes");
            return;
        }

        if (size > 128)
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("too much bytes");
            return;
        }

        if (address >= program_memory.size())
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Outside of available memory");
            return;
        }

        uint8_t *begin = program_memory.data() + address;
        uint32_t available_space_in_mem = program_memory.end() - begin;
        uint32_t count = std::min(available_space_in_mem, size);

        tx_data_frame.Push("OK");
        tx_data_frame.PushHex(begin, count);
    }

    void ProgMemClear(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {

        osStatus_t os_status = osMutexAcquire(program_memory_write_mutHandle, 0);

        if (os_status != osOK)
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("Cannot Access PROGMEM");
            return;
        }

        for (uint8_t &mem : program_memory)
        {
            mem = 0;
        }

        os_status = osMutexRelease(program_memory_write_mutHandle);

        tx_data_frame.Push("OK");
    }

    template <uint32_t N>
    static void PushTimeToFrame(DataFrame &tx_data_frame, Performance::Time<N>& timer, const char *const label)
    {
        constexpr uint32_t buff_size = 100;
        char buff[buff_size];

        Performance::Metrics time;
        time = timer.GetMetrics();

        snprintf(buff, buff_size, "%s c:%d m:%.9g v:%.9g", label, time.count, time.mean, time.variance);
        tx_data_frame.Push((const char *const)buff);
    }

    void Performance(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {

        tx_data_frame.Push("OK");

        PushTimeToFrame(tx_data_frame, Performance::module_update_time, "MOD_UPDATE");
        PushTimeToFrame(tx_data_frame, Performance::program_execution_time, "EXEC");
        PushTimeToFrame(tx_data_frame, Performance::program_first_scan_time, "EXEC_FS");
        PushTimeToFrame(tx_data_frame, Performance::start_sequence_time, "START_SEQ");
        PushTimeToFrame(tx_data_frame, Performance::stop_sequence_time, "STOP_SEQ");
        PushTimeToFrame(tx_data_frame, Performance::requests_time, "REQ");
    }

}
