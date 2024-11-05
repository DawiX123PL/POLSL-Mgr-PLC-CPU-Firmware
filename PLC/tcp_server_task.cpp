// #include <string>
// #include <boost/circular_buffer.hpp>
#include <inttypes.h>
#include "data_frame.hpp"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "lwip/sockets.h"
#include "static_buffer.hpp"

#include "tcp_command_handlers.hpp"

DataFrame rx_data_frame;
DataFrame tx_data_frame;

void HandleCommand(int client_socket)
{

    tx_data_frame.Clear();

    bool is_ok = rx_data_frame.Parse();

    if (!is_ok)
    {
        tx_data_frame.Clear();
        tx_data_frame.Push("PARSING_ERROR");
        tx_data_frame.Push("ERROR");

        // send response
        std::string_view resp = tx_data_frame.BufferGet();
        write(client_socket, resp.begin(), resp.size());
        return;
    }

    // first element of frame must be command name
    tx_data_frame.Clear();
    tx_data_frame.Push(rx_data_frame[0].Get<std::string_view>());

    std::string_view command = rx_data_frame[0].Get<std::string_view>();
    if (command == "PING")
    {
        TcpCommandHandle::Ping(rx_data_frame, tx_data_frame);
    }
    else if (command == "START")
    {
        TcpCommandHandle::Start(rx_data_frame, tx_data_frame);
    }
    else if (command == "STOP")
    {
        TcpCommandHandle::Stop(rx_data_frame, tx_data_frame);
    }
    else if (command == "PROGMEM")
    {
        TcpCommandHandle::ProgMem(rx_data_frame, tx_data_frame);
    }
    else if (command == "PERFORMANCE")
    {
        TcpCommandHandle::Performance(rx_data_frame, tx_data_frame);
    }
    else
    {
        TcpCommandHandle::UnnownCommand(rx_data_frame, tx_data_frame);
    }

    // send response
    std::string_view resp = tx_data_frame.BufferGet();
    write(client_socket, resp.begin(), resp.size());
}

void HandleConnectionLoop(int client_socket)
{
    while (true)
    {
        // read data from socket
        constexpr int buffer_size = 64;
        char buffer[buffer_size];
        int data_count = read(client_socket, buffer, buffer_size);

        if (data_count == 0)
        {
            close(client_socket);
            break; // client disconnected
        }

        // push data to buffer
        for (int i = 0; i < data_count; i++)
        {
            if (rx_data_frame.BufferFull())
                break;

            rx_data_frame.BufferPush(buffer[i]);
            if (buffer[i] == '\n')
            {
                HandleCommand(client_socket);
                rx_data_frame.Clear();
            }
        }

        // check for buffer overflow
        if (rx_data_frame.BufferFull())
        {
            // disconnect
            close(client_socket);
            break;
        }
    }
}

// thx
// https://medium.com/@coderx_15963/basic-tcp-ip-networking-in-c-using-posix-9a074d65bb35
//
extern "C" void TCPServerTaskFcn(void *argument)
{

    osDelay(1000); // wait for LWIP to startup;

    constexpr uint16_t tcp_ip_port = 2000;
    constexpr uint8_t max_incomming_connections = 4;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0); // create socket

    // error - could not create server socket
    while (server_socket)
    {
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(tcp_ip_port);
    address.sin_addr.s_addr = INADDR_ANY;

    // bind socket to server address
    int bind_result = bind(server_socket, (struct sockaddr *)&address, sizeof(address));

    // error - could not bind socket
    while (bind_result)
    {
    }

    while (true)
    {
        // wait for incomming connection
        listen(server_socket, max_incomming_connections);

        // accept incomming connection
        int client_socket = accept(server_socket, NULL, NULL);

        if (client_socket < 0)
        {
            // failed to accept connection
            continue;
        }

        HandleConnectionLoop(client_socket);
    }
}
