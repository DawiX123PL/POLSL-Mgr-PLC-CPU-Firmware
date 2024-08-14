
#include "tcp_command_handlers.hpp"
#include <array>
#include <algorithm>
#include "plc_user_code.hpp"
#include "plc_crc.hpp"

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
        tx_data_frame.Push("OK");
    }

    void Stop(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {
        tx_data_frame.Push("OK");
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
        CodeBlock *code_block = (CodeBlock *)program_memory.data();
        CodeBlockData *code_block_data = &code_block->code;

        uint32_t crc = code_block->crc;
        uint32_t crc_calculated = Crc32CalculateSoft((uint8_t *)code_block_data, sizeof(CodeBlockData));

        if (crc == crc_calculated)
        {
            tx_data_frame.Push("OK");
            return;
        }
        else
        {
            tx_data_frame.Push("ERROR");
            tx_data_frame.Push("CRC Mismatch");
            tx_data_frame.Push(crc);
            tx_data_frame.Push(crc_calculated);
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

        uint8_t *mem_ptr = &program_memory[address];
        uint8_t *mem_end = program_memory.end();
        uint32_t mem_count = mem_end - mem_ptr;

        uint32_t readed_bytes;
        bool is_hex_ok = rx_data_frame[3].GetHex(mem_ptr, mem_count, &readed_bytes);

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

        uint8_t *begin = program_memory.begin() + address;
        uint32_t available_space_in_mem = program_memory.end() - begin;
        uint32_t count = std::min(available_space_in_mem, size);

        tx_data_frame.Push("OK");
        tx_data_frame.PushHex(begin, count);
    }

    void ProgMemClear(const DataFrame &rx_data_frame, DataFrame &tx_data_frame)
    {

        for (uint8_t &mem : program_memory)
        {
            mem = 0;
        }

        tx_data_frame.Push("OK");
    }

}
