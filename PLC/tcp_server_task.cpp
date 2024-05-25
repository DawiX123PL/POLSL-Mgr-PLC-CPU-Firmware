
#include "FreeRTOS.h"
#include "cmsis_os.h"

extern "C" void TCPServerTaskFcn(void *argument)
{
    /* USER CODE BEGIN TCPServerTaskFcn */
    /* Infinite loop */
    for (;;)
    {
        osDelay(1);
    }
    /* USER CODE END TCPServerTaskFcn */
}
