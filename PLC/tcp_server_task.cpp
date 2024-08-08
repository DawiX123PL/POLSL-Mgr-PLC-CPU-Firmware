// #include <string>
// #include <boost/circular_buffer.hpp>
#include <inttypes.h>
#include "data_frame.hpp"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "lwip/sockets.h"
#include "static_buffer.hpp"




StaticBuffer<char, 1024> tcp_command;

void HandleCommand(int client_socket)
{
    DataFrame data_frame;
    DataFrame response_frame;
    response_frame.Clear();

    data_frame.Parse(tcp_command.Get(), tcp_command.Size());

    // first element of frame must be command name
    response_frame[0] = data_frame[0];

    if (data_frame[0].Get<std::string_view>() == "PING")
    {
        response_frame[1].Set("OK");
        std::string resp = response_frame.ToString();
        write(client_socket, resp.c_str(), resp.size());
        return;
    }

    // error case 
    {
        response_frame[1].Set("ERROR");
        response_frame[2].Set("Unnown command");
        std::string resp = response_frame.ToString();
        write(client_socket, resp.c_str(), resp.size());
        return;
    }
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
            if (tcp_command.Full())
                break;

            tcp_command.PushBack(buffer[i]);
            if (buffer[i] == '\n')
            {
                HandleCommand(client_socket);
                tcp_command.Clear();
            }
        }

        // check for buffer overflow
        if (tcp_command.Full())
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
