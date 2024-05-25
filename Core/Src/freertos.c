/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for DefaultTask */
osThreadId_t DefaultTaskHandle;
uint32_t SystemInitTaskBuffer[ 1024 ];
osStaticThreadDef_t SystemInitTaskControlBlock;
const osThreadAttr_t DefaultTask_attributes = {
  .name = "DefaultTask",
  .cb_mem = &SystemInitTaskControlBlock,
  .cb_size = sizeof(SystemInitTaskControlBlock),
  .stack_mem = &SystemInitTaskBuffer[0],
  .stack_size = sizeof(SystemInitTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TCPServerTask */
osThreadId_t TCPServerTaskHandle;
uint32_t TCPServerTaskBuffer[ 1024 ];
osStaticThreadDef_t TCPServerTaskControlBlock;
const osThreadAttr_t TCPServerTask_attributes = {
  .name = "TCPServerTask",
  .cb_mem = &TCPServerTaskControlBlock,
  .cb_size = sizeof(TCPServerTaskControlBlock),
  .stack_mem = &TCPServerTaskBuffer[0],
  .stack_size = sizeof(TCPServerTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void DefaultTaskFcn(void *argument);
void TCPServerTaskFcn(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of DefaultTask */
  DefaultTaskHandle = osThreadNew(DefaultTaskFcn, NULL, &DefaultTask_attributes);

  /* creation of TCPServerTask */
  TCPServerTaskHandle = osThreadNew(TCPServerTaskFcn, NULL, &TCPServerTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_DefaultTaskFcn */
/**
  * @brief  Function implementing the DefaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_DefaultTaskFcn */
void DefaultTaskFcn(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN DefaultTaskFcn */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DefaultTaskFcn */
}

/* USER CODE BEGIN Header_TCPServerTaskFcn */
/**
* @brief Function implementing the TCPServerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TCPServerTaskFcn */
__weak void TCPServerTaskFcn(void *argument)
{
  /* USER CODE BEGIN TCPServerTaskFcn */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END TCPServerTaskFcn */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

