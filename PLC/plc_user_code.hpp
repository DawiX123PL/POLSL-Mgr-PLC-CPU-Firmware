#pragma once
#include <inttypes.h>
#include <array>
#include <cmsis_os2.h>
#include "plc_crc.hpp"

extern "C"
{
    constexpr uint32_t user_code_bin_size = 40 * 1024; // program memory - 40kB
    constexpr uint32_t modules_count = 64;             // program memory - 40kB

    typedef void (*ProgramPtr)(void *, void *);
    typedef void (*ProgramInitPtr)(void *, void *);

    // struct containing pointers to executable functions and program size
    struct SymbolTable
    {
        ProgramPtr main_program_relptr;          // pointer relative to beginning of symbol table
        ProgramInitPtr main_program_init_relptr; // pointer relative to beginning of symbol table
        uint32_t program_size;
    };

    struct CodeBlockData
    {
        // info about physical modules required by user code
        uint8_t required_modules[modules_count] = {};

        // executable code
        union
        {
            SymbolTable symbol_table;
            uint8_t executable_code_bin[user_code_bin_size] = {};
        };
    };

    struct CodeBlock
    {
        uint32_t crc; // to verify user code integrity;
        CodeBlockData code;

        bool Verify()
        {
            uint32_t crc_calculated = Crc32CalculateSoft((uint8_t *)&code, sizeof(CodeBlockData));
            return crc == crc_calculated;
        }

        bool Verify(uint32_t *code_crc, uint32_t *calculated_crc)
        {
            uint32_t crc_calc = Crc32CalculateSoft((uint8_t *)&code, sizeof(CodeBlockData));
            *code_crc = crc;
            *calculated_crc = crc_calc;
            return crc == crc_calc;
        }

        size_t size()
        {
            return sizeof(CodeBlock);
        }

        uint8_t *data()
        {
            return (uint8_t *)this;
        }

        uint8_t *begin()
        {
            return (uint8_t *)this;
        }

        uint8_t *end()
        {
            return begin() + size();
        }
    };



    struct GlobalMem
    {
    	static constexpr int max_io_modules = 5;
    	static constexpr int bytes_per_module = 64;
    	static constexpr int memory_size = 1024;

        uint8_t input[max_io_modules][bytes_per_module];  // %Ix.x.x
        uint8_t output[max_io_modules][bytes_per_module]; // %Qx.x.x
        uint8_t memory[memory_size]; // %Mx.x
    } __attribute__((packed));
}

template <typename PointerType>
PointerType RelativeToGlobalPointer(SymbolTable* table, PointerType relative_ptr)
{
    // step 1
    // pointer to program relative to symbol table
    ptrdiff_t pointer_diff = (uint8_t *)relative_ptr - (uint8_t *)nullptr;

    // pointer to program relative to global address space
    uint8_t *global_pointer = (uint8_t *)table + pointer_diff;

    return (PointerType)global_pointer;
}

// this mutex protects against unwanted write to program memory
extern osMutexId_t program_memory_write_mutHandle;
extern CodeBlock program_memory;
