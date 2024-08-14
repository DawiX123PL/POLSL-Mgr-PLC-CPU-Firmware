#pragma once

#include <string>
#include <inttypes.h>
#include <charconv>
#include <cstring>

// template<class StringType = std::string_view>
class DataFrame
{
public:
    class Data
    {
        std::string_view str;

    public:
        // true if no error occured
        template <typename T>
        bool GetIfExist(T *element) const
        {
            std::from_chars_result res = std::from_chars(str.begin(), str.end(), *element);
            // check if whole string was consumed
            bool whole_str_parsed = res.ptr == str.end();
            bool err = res.ec == std::errc{};
            return whole_str_parsed && err;
        }

        template <typename T>
        T Get() const
        {
            T element = {};
            GetIfExist(&element);
            return element;
        }

        bool GetHex(uint8_t *const array, uint32_t array_size, uint32_t *received_bytes) const;

        void Clear();
        uint32_t AllocatedSize();

        friend class DataFrame;
    };

private:
    // buffer containing text
    static constexpr uint32_t buffer_capacity = 1024;
    uint32_t buffer_size = 0;
    char buffer[buffer_capacity];

    // buffer containing datablocks
    static constexpr uint32_t datablocks_capacity = 16;
    uint32_t datablock_size = 0;
    Data datablocks[datablocks_capacity] = {};

public:
    DataFrame() : buffer_size(0), datablock_size(0) {};

    // return -1 in case of error
    bool Parse();
    const Data &operator[](uint32_t index) const;

    void Clear();
    uint32_t Size();

    template <typename T>
    bool Push(const T &element)
    {
        uint32_t old_buffer_size = buffer_size;

        bool is_first_block = buffer_size == 0;
        char *separator_ptr;
        char *begin;

        if (is_first_block)
        {
            separator_ptr = nullptr;
            begin = buffer;
        }
        else
        {
            separator_ptr = &buffer[buffer_size - 1]; // this must be later changed to ';'
            begin = separator_ptr + 1; // byte after separator
        }

        char *buffer_end = &buffer[buffer_capacity - 1];

        // sanity checks
        if (begin >= &buffer[buffer_capacity])
        {
            return false;
        }

        // convert to chars
        std::to_chars_result result = std::to_chars(begin, buffer_end, element);
        char *last_char = result.ptr;
        if (result.ec != std::errc())
        {
            return false;
        }

        // calculate new size
        buffer_size = last_char - buffer;

        // push newline at end
        bool is_ok = BufferPush('\n');
        if (!is_ok)
        {
            // revert changes in case of error
            buffer_size = old_buffer_size;
            return false;
        }

        if (datablock_size >= datablocks_capacity)
        {
            return false;
        }

        datablocks[datablock_size].str = std::string_view(begin, last_char - begin);
        datablock_size++;

        if (!is_first_block && separator_ptr != nullptr)
        {
            *separator_ptr = ';';
        }

        return true;
    }

    bool Push(const char *const element);
    bool PushHex(const uint8_t *const array, uint32_t array_size);

    //************
    void BufferClear();
    bool BufferFull();
    bool BufferEmpty();
    uint32_t BufferSize();
    bool BufferPush(char c);
    std::string_view BufferGet();
};

//**********************************************************************

template <>
bool DataFrame::Data::GetIfExist(std::string *e) const;

template <>
bool DataFrame::Data::GetIfExist(std::string_view *e) const;

template <>
bool DataFrame::Data::GetIfExist(const char **const e) const;

//**********************************************************************

template <>
std::string DataFrame::Data::Get() const;

template <>
std::string_view DataFrame::Data::Get() const;

template <>
const char *const DataFrame::Data::Get() const;

//**********************************************************************

template <>
bool DataFrame::Push(const std::string_view &element);

template <>
bool DataFrame::Push(const std::string &element);
