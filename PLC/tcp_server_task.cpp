
#include <inttypes.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "lwip/sockets.h"

// thx
// https://medium.com/@coderx_15963/basic-tcp-ip-networking-in-c-using-posix-9a074d65bb35
//
extern "C" void TCPServerTaskFcn(void *argument)
{

	osDelay(100); // wait for LWIP to startup;

    constexpr uint16_t tcp_ip_port = 2000;
    constexpr uint8_t max_incomming_connections = 4;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(tcp_ip_port);
    address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&address, sizeof(address));

    for (;;)
    {
        listen(server_socket, max_incomming_connections);

        int client_socket = accept(server_socket, NULL, NULL);

        char message[] = "Hello, World!";

//         Send message to the client
        send(client_socket, message, strlen(message), 0);

        osDelay(10000);

        // Close the client socket
        close(client_socket);


    }
    /* USER CODE END TCPServerTaskFcn */
}
