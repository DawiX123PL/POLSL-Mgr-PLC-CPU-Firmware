#pragma once
#include <inttypes.h>
#include <array>

extern "C"
{
    constexpr uint32_t user_code_bin_size = 20 * 1024; // program memory - 20kB
    constexpr uint32_t modules_count = 64;             // program memory - 20kB

    typedef void (*ProgramPtr)(void *);
    typedef void (*ProgramInitPtr)(void *);

    // struct containing pointers to executable functions and program size
    struct SymbolTable
    {
        ProgramPtr program;
        ProgramInitPtr program_init;
        uint32_t program_size;
    };

    struct CodeBlockData
    {
        // info about physical modules required by user code
        uint8_t required_modules[modules_count] = {};

        // executable code
        uint8_t executable_code[user_code_bin_size] = {};
    };

    struct CodeBlock
    {
        uint32_t crc; // to verify user code integrity;
        CodeBlockData code;
    };
}

extern std::array<uint8_t, sizeof(CodeBlock)> program_memory;
