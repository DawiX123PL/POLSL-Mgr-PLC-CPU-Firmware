#include "performance.hpp"

namespace Performance
{

    uint32_t previous_time_stamp;

    void StartTimer()
    {
        HAL_TIM_Base_Start(&htim_performance);
        previous_time_stamp = __HAL_TIM_GET_COUNTER(&htim_performance);
    }

    // timer frequency 84 MHz
    uint32_t GetElapsedTime()
    {
        uint32_t current_time_stamp = __HAL_TIM_GET_COUNTER(&htim_performance);
        uint32_t elapsed_time = current_time_stamp - previous_time_stamp;
        previous_time_stamp = current_time_stamp;
        return elapsed_time;
    }

    Time<16> module_update_time;
    Time<16> program_execution_time;
    Time<16> program_first_scan_time;
    Time<16> start_sequence_time;
    Time<16> stop_sequence_time;
    Time<16> requests_time;

    void InitTimers()
    {
        module_update_time.Init();
        program_execution_time.Init();
        program_first_scan_time.Init();
        start_sequence_time.Init();
        stop_sequence_time.Init();
        requests_time.Init();
    }

}
