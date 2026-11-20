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
#include "App/system_state.hpp"
#include "FreeRTOS.h"
#include "custom_algobuild.h"
#include "App/settings.h"
#include "App/utilities.hpp"

#include "EPD_Test.h"
#include "ab_sensor_hub.h"
#include "cmsis_os2.h"
#include "algo_builder.h"
#include "L76X.h"
#include "gui.hpp"
#include "io.hpp"
#include "main.h"
#include "portmacrocommon.h"
#include "projdefs.h"
#include "stm32u3xx_hal_def.h"
#include "stm32u3xx_hal_gpio.h"

#include <string.h>
#include <atomic>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */
using gui::GUI;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern UART_HandleTypeDef hcom_uart[COMn];
extern uint32_t Update100Hz;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
Button button1(ButtonId::B1);
Button button2(ButtonId::B2);
Button button3(ButtonId::B3);

osThreadId_t ComExportTaskHandle;
uint32_t ComExportTaskBuffer[ COM_EXPORT_TASK_STACK_SIZE ];
osStaticThreadDef_t ComExportTaskControlBloc;
const osThreadAttr_t ComExportTask_attributes = {
  .name = "ComExportTask",
  .cb_mem = &ComExportTaskControlBloc,
  .cb_size = sizeof(ComExportTaskControlBloc),
  .stack_mem = &ComExportTaskBuffer[0],
  .stack_size = sizeof(ComExportTaskBuffer),
  .priority = COM_EXPORT_TASK_PRIO
};

osThreadId_t ComDebugTaskHandle;
uint32_t ComDebugTaskBuffer[ COM_DEBUG_TASK_STACK_SIZE ];
osStaticThreadDef_t ComDebugTaskControlBloc;
const osThreadAttr_t ComDebugTask_attributes = {
  .name = "ComDebugTask",
  .cb_mem = &ComDebugTaskControlBloc,
  .cb_size = sizeof(ComDebugTaskControlBloc),
  .stack_mem = &ComDebugTaskBuffer[0],
  .stack_size = sizeof(ComDebugTaskBuffer),
  .priority = COM_DEBUG_TASK_PRIO
};

osThreadId_t SensorHubTaskHandle;
uint32_t SensorHubTaskBuffer[ SENSOR_HUB_TASK_STACK_SIZE ];
osStaticThreadDef_t SensorHubTaskControlBloc;
const osThreadAttr_t SensorHubTask_attributes = {
  .name = "SensorHubTask",
  .cb_mem = &SensorHubTaskControlBloc,
  .cb_size = sizeof(SensorHubTaskControlBloc),
  .stack_mem = &SensorHubTaskBuffer[0],
  .stack_size = sizeof(SensorHubTaskBuffer),
  .priority = SENSOR_HUB_TASK_PRIO
};

osThreadId_t GNSSTaskHandle;
uint32_t GNSSTaskBuffer[ GNSS_TASK_STACK_SIZE ];
osStaticThreadDef_t GNSSTaskControlBloc;
const osThreadAttr_t GNSSTask_attributes = {
  .name = "GNSSTask",
  .cb_mem = &GNSSTaskControlBloc,
  .cb_size = sizeof(GNSSTaskControlBloc),
  .stack_mem = &GNSSTaskBuffer[0],
  .stack_size = sizeof(GNSSTaskBuffer),
  .priority = GNSS_TASK_PRIO
};

osThreadId_t DisplayTaskHandle;
uint32_t DisplayTaskBuffer[ DISPLAY_TASK_STACK_SIZE ];
osStaticThreadDef_t DisplayTaskControlBloc;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .cb_mem = &DisplayTaskControlBloc,
  .cb_size = sizeof(DisplayTaskControlBloc),
  .stack_mem = &DisplayTaskBuffer[0],
  .stack_size = sizeof(DisplayTaskBuffer),
  .priority = DISPLAY_TASK_PRIO
};

osThreadId_t InputManagerTaskHandle;
uint32_t InputManagerTaskBuffer[ INPUT_MANAGER_TASK_STACK_SIZE ];
osStaticThreadDef_t InputManagerTaskControlBloc;
const osThreadAttr_t InputManagerTask_attributes = {
  .name = "InputManagerTask",
  .cb_mem = &InputManagerTaskControlBloc,
  .cb_size = sizeof(InputManagerTaskControlBloc),
  .stack_mem = &InputManagerTaskBuffer[0],
  .stack_size = sizeof(InputManagerTaskBuffer),
  .priority = INPUT_MANAGER_TASK_PRIO
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
    .cb_mem = &SystemTaskControlBloc,
  .cb_size = sizeof(SystemTaskControlBloc),
  .stack_mem = &SystemTaskBuffer[0],
  .stack_size = sizeof(SystemTaskBuffer),
  .priority = SYSTEM_TASK_PRIO
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void StartComExportTask(void *argument);
void StartComDebugTask(void *argument);
void StartSensorHubTask(void *argument);
void StartGNSSTask(void *argument);
void StartDisplayTask(void *argument);
void StartInputManagerTask(void *argument);

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
  SystemState& system_state = SystemState::getInstance();
  (void)system_state; // unused here

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
  ComExportTaskHandle = osThreadNew(StartComExportTask, NULL, &ComExportTask_attributes);
  ComDebugTaskHandle = osThreadNew(StartComDebugTask, NULL, &ComDebugTask_attributes);
  SensorHubTaskHandle = osThreadNew(StartSensorHubTask, NULL, &SensorHubTask_attributes);
  GNSSTaskHandle = osThreadNew(StartGNSSTask, NULL, &GNSSTask_attributes);
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);
  InputManagerTaskHandle = osThreadNew(StartInputManagerTask, NULL, &InputManagerTask_attributes);

  configASSERT(ComDebugTaskHandle != NULL);
  configASSERT(SensorHubTaskHandle != NULL);
  configASSERT(GNSSTaskHandle != NULL);
  configASSERT(DisplayTaskHandle != NULL);
  configASSERT(InputManagerTaskHandle != NULL);
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
    /* Magnetometer management */
    if (sensor_hub_data.mfx_magcal.cal_quality <  MFX_MAGCALOK)
    {
      sensor_hub_data.motion_fx_6x_enable = 1;
      sensor_hub_data.motion_fx_9x_enable = 0;
    } else
    {
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

  SystemState& system_state = SystemState::getInstance();

  for (;;)
  {
    Update100Hz = 1U;
    Time_Handler();
    AB_Handler();
    
    system_state.acquire();
    system_state.updateState();
    system_state.release();
    
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}

void StartGNSSTask(void *argument)
{
  GPSdata_t gps{};
  SystemState& system_state = SystemState::getInstance();

  for (;;)
  {
    L76X_Receive_NMEA(&gps); // Blocking
    Coordinates google_coors = L76X_Google_Coordinates(&gps);
    DMS_Coordinates dms_coors = Google2DMS(google_coors, gps.Lat_area, gps.Lon_area);

    if (gps.Status == GPS_Status::OK) {
      system_state.acquire();
      SystemState_t new_state = system_state.getState();

      GNSS_State_t gnss {};
      gnss.coors = dms_coors;
      gnss.time = gps.Time;
      gnss.speed = gps.Speed;
      gnss.date = gps.Date;

      new_state.gnss = gnss;

      system_state.setState(new_state);
      system_state.release();
    }
  }
}

void StartDisplayTask(void *argument)
{
  GUI& gui = GUI::getInstance();

  for (;;)
  {
    if (xTaskNotifyWait(
      0,
      UINT32_MAX,
      NULL,
      pdMS_TO_TICKS(DISPLAY_REFRESH_RATE_MS)
    )
    == pdTRUE)  /* Received button presses */
  {
      gui.processInput();
      gui.refreshScreen();
    }
    else  /* Didn't recieve button pressed BUT screen needs to be refreshed */
    {
      gui.refreshScreen();
    }
  }
}

void StartInputManagerTask(void *argument)
{
  uint32_t isr_flags;
  std::array<Button*, 3> button_list = {&button1, &button2, &button3};

  for (;;)
  {
    /* Wait for interrupt or after timeout poll the button state anyways */
    xTaskNotifyWait(
        0,
        UINT32_MAX,
        &isr_flags,
        pdMS_TO_TICKS(5)
    );

    bool actionDetected = false;

    /* Handle the state of each button */
    for (uint8_t i = 0; i < button_list.size(); i++) {
      bool isActive = (isr_flags & (1 << i)) >> i;
      handleButtonEvent(*button_list.at(i), isActive, &actionDetected);
    }

    if (actionDetected) {
      /* Send a refresh request to the display manager*/
      xTaskNotify(
        reinterpret_cast<TaskHandle_t>(DisplayTaskHandle),
        0,
        eNoAction
      );
    }
  }
}

void StartComExportTask(void *argument)
{
  SystemState& system_state = SystemState::getInstance();

  // magic word: EXPORT_TLM
  etl::string<TELEMETRY_MSG_SIZE> msg = "(Received something on UART)\r\n";

  for (;;)
  {
    xTaskNotifyWait(
      0,
      UINT32_MAX,
      NULL,
      HAL_MAX_DELAY
    );
    
    HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)msg.c_str(), strlen(msg.c_str()), HAL_MAX_DELAY);
  }
}
/* USER CODE END Application */
