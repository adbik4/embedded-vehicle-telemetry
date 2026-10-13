/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
#include "settings.h"
#include "utilities.h"
#include "custom_algobuild.h"
#include "algo_builder.h"

#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern UART_HandleTypeDef hcom_uart[COMn];
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t ComDebugTaskHandle;
uint32_t ComDebugTaskBuffer[ COM_DEBUG_TASK_STACK_SIZE ];
osStaticThreadDef_t ComDebugTaskControlBloc;
const osThreadAttr_t ComDebugTask_attributes = {
  .name = "ComDebugTask",
  .stack_mem = &ComDebugTaskBuffer[0],
  .stack_size = sizeof(ComDebugTaskBuffer),
  .cb_mem = &ComDebugTaskControlBloc,
  .cb_size = sizeof(ComDebugTaskControlBloc),
  .priority = COM_DEBUG_TASK_PRIO,
};

osThreadId_t SensorHubTaskHandle;
uint32_t SensorHubTaskBuffer[ SENSOR_HUB_TASK_STACK_SIZE ];
osStaticThreadDef_t SensorHubTaskControlBloc;
const osThreadAttr_t SensorHubTask_attributes = {
  .name = "SensorHubTask",
  .stack_mem = &SensorHubTaskBuffer[0],
  .stack_size = sizeof(SensorHubTaskBuffer),
  .cb_mem = &SensorHubTaskControlBloc,
  .cb_size = sizeof(SensorHubTaskControlBloc),
  .priority = SENSOR_HUB_TASK_PRIO,
};

/* Definitions for ComDebugQueue */
osMessageQueueId_t ComDebugQueueHandle;
uint8_t ComDebugQueueBuffer[DEBUG_QUEUE_LENGTH * DEBUG_MSG_SIZE];
osStaticMessageQDef_t ComDebugQueueControlBlock;
const osMessageQueueAttr_t ComDebugQueue_attributes = {
  .name = "ComDebugQueue",
  .cb_mem = &ComDebugQueueControlBlock,
  .cb_size = sizeof(ComDebugQueueControlBlock),
  .mq_mem = &ComDebugQueueBuffer,
  .mq_size = sizeof(ComDebugQueueBuffer)
};
/* USER CODE END Variables */
/* Definitions for systemTask */
osThreadId_t systemTaskHandle;
uint32_t SystemTaskBuffer[ SYSTEM_TASK_STACK_SIZE ];
osStaticThreadDef_t SystemTaskControlBloc;
const osThreadAttr_t systemTask_attributes = {
  .name = "systemTask",
  .stack_mem = &SystemTaskBuffer[0],
  .stack_size = sizeof(SystemTaskBuffer),
  .cb_mem = &SystemTaskControlBloc,
  .cb_size = sizeof(SystemTaskControlBloc),
  .priority = SYSTEM_TASK_PRIO,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void StartComDebugTask(void *argument);
void StartSensorHubTask(void *argument);

/* USER CODE END FunctionPrototypes */

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

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
  ComDebugQueueHandle = osMessageQueueNew (DEBUG_QUEUE_LENGTH, DEBUG_MSG_SIZE, &ComDebugQueue_attributes);
  /* USER CODE END RTOS_QUEUES */
  /* creation of systemTask */
  systemTaskHandle = osThreadNew(StartSystemTask, NULL, &systemTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  ComDebugTaskHandle = osThreadNew(StartComDebugTask, NULL, &ComDebugTask_attributes);
  SensorHubTaskHandle = osThreadNew(StartSensorHubTask, NULL, &SensorHubTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartSystemTask */
/**
* @brief Function implementing the systemTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSystemTask */
void StartSystemTask(void *argument)
{
  /* USER CODE BEGIN systemTask */
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(1000);

  /* Infinite loop */
  for(;;)
  {
    // TODO: Tell the screen to refresh every second

    /* Magnetometer management */
    if (sensor_hub_data.mfx_magcal.cal_quality <  MFX_MAGCALOK)
    {
      PRINT_DEBUG("FUSION 6X\r\n");
      sensor_hub_data.motion_fx_6x_enable = 1;
      sensor_hub_data.motion_fx_9x_enable = 0;
    } else
    {
      PRINT_DEBUG("FUSION 9X\r\n");
      sensor_hub_data.motion_fx_6x_enable = 0;
      sensor_hub_data.motion_fx_9x_enable = 1;
    }

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
  /* USER CODE END systemTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartComDebugTask(void *argument)
{
  char msg[DEBUG_MSG_SIZE];

  for(;;)
  {
    /* Keep waiting for messages to display */
    if (osMessageQueueGet(ComDebugQueueHandle, msg, NULL, 1000) == osOK)
        {
            HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)&msg, strlen(msg), HAL_MAX_DELAY);
        }
  }
}

void StartSensorHubTask(void *argument)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xPeriod = pdMS_TO_TICKS(10);

  for (;;)
  {
    Time_Handler();
    AB_Handler();

    PRINT_DEBUG(
      "t %d, acc_x %f, acc_y %f, acc_z %f\r\n",
      (int)sensor_hub_data.timestamp_us,
      sensor_hub_data.acceleration[0],
      sensor_hub_data.acceleration[1],
      sensor_hub_data.acceleration[2]
    );
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}
/* USER CODE END Application */

