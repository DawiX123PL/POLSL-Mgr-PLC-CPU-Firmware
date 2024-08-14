#include "data_frame.hpp"
#include <math.h>

//**********************************************************************
// Dataframe methods

bool DataFrame::Parse()
{

    int db_number = 0;
    int prev_index = 0;
    const char *prev_ptr = buffer;

    for (uint32_t i = 0; i < buffer_size; i++)
    {
        char c = buffer[i];
        if (c == ';' || c == '\n')
        {
            datablocks[db_number].str = std::string_view(prev_ptr, i - prev_index);

            prev_ptr = buffer + i + 1;
            prev_index = i + 1;
            db_number++;
            if (c == '\n')
            {
                buffer_size = i + 1; // remove all chars after end of command
                datablock_size = db_number;
                return true;
            }
        }
    }

    return false;
}

const DataFrame::Data &DataFrame::operator[](uint32_t index) const
{
    uint32_t bounded_index = std::min(index, datablocks_capacity - 1);
    bounded_index = std::min(bounded_index, datablock_size - 1);

    return datablocks[bounded_index];
}

void DataFrame::Clear()
{
    datablock_size = 0;
    BufferClear();
}

uint32_t DataFrame::Size()
{
    return datablock_size;
}

//************
void DataFrame::BufferClear()
{
    buffer_size = 0;
}
uint32_t DataFrame::BufferSize()
{
    return buffer_size;
}
bool DataFrame::BufferFull()
{
    return buffer_size == buffer_capacity;
}
bool DataFrame::BufferEmpty()
{
    return buffer_size == 0;
}

bool DataFrame::BufferPush(char c)
{
    if (buffer_size >= buffer_capacity)
    {
        return false;
    }

    buffer[buffer_size] = c;
    buffer_size++;
    return true;
}

std::string_view DataFrame::BufferGet()
{
    return std::string_view(buffer, buffer_size);
}

//**********************************************************************
// DataFrame::Data methods

void DataFrame::Data::Clear()
{
    str = ""; // assign empty string
}

//**********************************************************************
// DataFrame::Data conversion methods
// DataFrame::Data -> type

template <>
bool DataFrame::Data::GetIfExist(std::string *e) const
{
    *e = str;
    return true;
}

template <>
bool DataFrame::Data::GetIfExist(std::string_view *e) const
{
    *e = str;
    return true;
}

//**********************************************************************

template <>
std::string DataFrame::Data::Get() const
{
    return std::string(str);
}

template <>
std::string_view DataFrame::Data::Get() const
{
    return str;
}

//**********************************************************************
// DataFrame::Data conversion methods
// type -> DataFrame::Data

template <>
bool DataFrame::Push(const std::string_view &element)
{
    // check if new data will fit

    uint32_t space_after_changes = buffer_size + element.size() + 1;

    if (space_after_changes > buffer_capacity)
    {
        return false;
    }

    if (datablock_size >= datablocks_capacity)
    {
        return false;
    }

    if (buffer_size > 0)
    {
        buffer[buffer_size - 1] = ';';
    }

    char *begin = &buffer[buffer_size];

    for (char c : element)
    {
        BufferPush(c);
    }

    char *end = &buffer[buffer_size];

    datablocks[datablock_size].str = std::string_view(begin, end - begin);
    datablock_size++;

    return BufferPush('\n');
}

template <>
bool DataFrame::Push(const std::string &element)
{
    std::string_view sv = element;
    return Push(sv);
}

bool DataFrame::Push(const char *const element)
{
    std::string_view sv{element};
    return Push(sv);
}

//******************************************************************************

static uint8_t CharToNum(char c, bool &err)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 0x0A;
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 0x0A;
    }
    err = true;
    return 0;
}

static uint8_t CharPairToNum(char c1, char c2, bool &err)
{
    uint8_t n1 = CharToNum(c1, err);
    uint8_t n2 = CharToNum(c2, err);
    return (n1 << 4) + n2;
}

static char NumToChar(uint8_t num, bool &err)
{
    if (num >= 0 && num <= 9)
    {
        return '0' + num;
    }

    if (num >= 0x0A && num <= 0x0F)
    {
        return 'A' + num - 0x0A;
    }

    err = true;
    return '0';
}

static bool IsHexChar(char c)
{

    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static std::pair<char, char> NumToCharPair(uint8_t num)
{
    bool err;
    char c1 = NumToChar(num >> 4, err);
    char c2 = NumToChar(num & (0x0F), err);
    return {c1, c2};
}

bool DataFrame::PushHex(const uint8_t *const array, uint32_t array_size)
{
    // check if contains needed space
    uint32_t available_space = buffer_capacity - buffer_size;
    uint32_t needed_space = array_size * 2 + 1;

    if (available_space < needed_space)
        return false;

    if(datablock_size >= datablocks_capacity)
        return false;

    buffer[buffer_size - 1] = ';';

    char* begin = &buffer[buffer_size];

    for (uint32_t i = 0; i < array_size; i++)
    {
        auto chars = NumToCharPair(array[i]);
        BufferPush(chars.first);
        BufferPush(chars.second);
    }

    datablocks[datablock_size++].str = std::string_view(begin, array_size * 2); 

    BufferPush('\n');


    return true;
}

bool DataFrame::Data::GetHex(uint8_t *const array, uint32_t array_size, uint32_t *received_bytes) const
{
    // check for errors in string
    for (char c : str)
    {
        if (!IsHexChar(c))
        {
            *received_bytes = 0;
            return false;
        }
    }

    // begin conversion
    for (uint32_t index = 0; index < array_size; index++)
    {
        if (index * 2 + 1 > str.size())
        {
            *received_bytes = index;
            return true;
        }

        bool err;
        uint8_t num = CharPairToNum(str[index * 2], str[index * 2 + 1], err);
        array[index] = num;
    }

    *received_bytes = array_size;
    return true;
}
