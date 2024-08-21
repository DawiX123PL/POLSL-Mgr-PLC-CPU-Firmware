#pragma once
#include <inttypes.h>

namespace PlcRequestFlags
{
    static constexpr uint32_t NONE = 0;
    static constexpr uint32_t RUN = 1<<1;
    static constexpr uint32_t STOP = 1<<2;
};

enum class PlcStatus
{
    RUN,
    STOP,
    STARTING, // intemediate between start and stop
    STOPING, // intemediate between start and stop
};



