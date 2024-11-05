#pragma once

#include <inttypes.h>
#include "tim.h"
#include "cmsis_os2.h"

namespace Performance
{
    template <uint32_t N>
    class Accumulator
    {
        uint32_t data[N];
        uint8_t count = 0;
        uint8_t next_index = 0;

    public:
        void Push(uint32_t _data)
        {
            data[next_index] = _data;
            next_index++;
            if (next_index >= N)
            {
                next_index = 0;
            }
            if (count < N)
            {
                count++;
            }
        }

        float Mean()
        {
            uint64_t sum = 0;
            for (int i = 0; i < count; i++)
            {
                sum += data[i];
            }
            return (float)sum / count;
        }

        float Variance(float mean)
        {
            float sum_error = 0;
            for (int i = 0; i < count; i++)
            {
                float error = data[i] - mean;
                sum_error += error * error;
            }

            return sum_error / (count - 1);
        }

        uint8_t Size()
        {
            return count;
        }

        float Variance()
        {
            float mean = Mean();
            return Variance(mean);
        }
    };

    void StartTimer();

    // timer frequency 84 MHz
    uint32_t GetElapsedTime();

    struct Metrics
    {
        float mean = 0;
        float variance = 0;
        int count = 0;
    };

    template <uint32_t N>
    class Time
    {
        Accumulator<N> time_stamps;

        osMutexId_t mutex;


        // assuming timer runs with frequency 84MHz
        static constexpr float timer_frequency = 84000000.0f;

        // converts time accumulated by htim_performance [1] to time [s]
        inline float TimerToSeconds(float t)
        {
            return t / timer_frequency;
        }

        // converts timer variance [1] to time variance [s^2]
        inline float VarianceToSecondsSqr(float t)
        {
            return t / (timer_frequency * timer_frequency);
        }

    public:
        void Init()
        {
            const osMutexAttr_t mutex_attr = {
                "TimePrivLock",     // human readable mutex name
                osMutexPrioInherit, // attr_bits
                NULL,               // memory for control block
                0U                  // size for control block
            };
            mutex = osMutexNew(&mutex_attr);
        }

        void Update()
        {
            osStatus_t status = osMutexAcquire(mutex, 10);
            if (status == osOK)
            {
                time_stamps.Push(GetElapsedTime());
                osMutexRelease(mutex);
            }
        }

        Metrics GetMetrics()
        {
            osStatus_t status = osMutexAcquire(mutex, 0);
            Metrics m;
            if (status == osOK)
            {
                float mean_timer = time_stamps.Mean();

                m.mean = TimerToSeconds(mean_timer);
                m.variance = VarianceToSecondsSqr(time_stamps.Variance(mean_timer));
                m.count = time_stamps.Size();

                osMutexRelease(mutex);
            }
            return m;
        }
    };

    extern Time<16> module_update_time;
    extern Time<16> program_execution_time;
    extern Time<16> program_first_scan_time;
    extern Time<16> start_sequence_time;
    extern Time<16> stop_sequence_time;
    extern Time<16> requests_time;

    void InitTimers();

};
