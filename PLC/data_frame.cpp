#include "data_frame.hpp"
#include <math.h>

//**********************************************************************
// Dataframe methods

int DataFrame::Parse()
{
    Clear();

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
                return i;
            }
        }
    }

    return -1;
}

DataFrame::Data &DataFrame::operator[](uint32_t index)
{
    uint32_t bounded_index = std::min(index, datablocks_capacity - 1);
    bounded_index = std::min(bounded_index, datablock_size - 1);

    return datablocks[bounded_index];
}

void DataFrame::Clear()
{
    datablock_size = 0;
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
bool DataFrame::Data::GetIfExist(std::string *e)
{
    *e = str;
    return true;
}

template <>
bool DataFrame::Data::GetIfExist(std::string_view *e)
{
    *e = str;
    return true;
}

//**********************************************************************

template <>
std::string DataFrame::Data::Get()
{
    return std::string(str);
}

template <>
std::string_view DataFrame::Data::Get()
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

    buffer[buffer_size - 1] = ';';
    char * begin = &buffer[buffer_size]; 

    for (char c : element)
    {
        BufferPush(c);
    }

    char * end = &buffer[buffer_size]; 

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