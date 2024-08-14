#pragma once

#include "data_frame.hpp"

namespace TcpCommandHandle
{
    void UnnownCommand(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void Ping(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void Start(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void Stop(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void ProgMem(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void ProgMemWrite(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void ProgMemRead(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void ProgMemClear(const DataFrame& rx_data_frame, DataFrame &tx_data_frame);
    void ProgMemVerify(const DataFrame &rx_data_frame, DataFrame &tx_data_frame);

}
