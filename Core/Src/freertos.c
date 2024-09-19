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
#include "user_mb_app.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
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
/* Definitions for MasterTask */
osThreadId_t MasterTaskHandle;
const osThreadAttr_t MasterTask_attributes = {
  .name = "MasterTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Slave_Task */
osThreadId_t Slave_TaskHandle;
const osThreadAttr_t Slave_Task_attributes = {
  .name = "Slave_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MasterSendTask */
osThreadId_t MasterSendTaskHandle;
const osThreadAttr_t MasterSendTask_attributes = {
  .name = "MasterSendTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void MasterTaskFun(void *argument);
void Slave_TaskFun(void *argument);
void MasterSendTaskFun(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  eMBInit(MB_RTU, 0x01, 3, 9600, MB_PAR_NONE);
  eMBEnable();
  eMBMasterInit(MB_RTU, 2, 9600, MB_PAR_NONE);
  eMBMasterEnable();
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
  /* creation of MasterTask */
  MasterTaskHandle = osThreadNew(MasterTaskFun, NULL, &MasterTask_attributes);

  /* creation of Slave_Task */
  Slave_TaskHandle = osThreadNew(Slave_TaskFun, NULL, &Slave_Task_attributes);

  /* creation of MasterSendTask */
  MasterSendTaskHandle = osThreadNew(MasterSendTaskFun, NULL, &MasterSendTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_MasterTaskFun */
/**
  * @brief  Function implementing the MasterTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_MasterTaskFun */
void MasterTaskFun(void *argument)
{
  /* USER CODE BEGIN MasterTaskFun */
  /* Infinite loop */
  for(;;)
  {
   eMBMasterPoll();
  }
  /* USER CODE END MasterTaskFun */
}

/* USER CODE BEGIN Header_Slave_TaskFun */
/**
* @brief Function implementing the Slave_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Slave_TaskFun */
__weak void Slave_TaskFun(void *argument)
{
  /* USER CODE BEGIN Slave_TaskFun */
  /* Infinite loop */
  for(;;)
  {
    eMBPoll();
  }
  /* USER CODE END Slave_TaskFun */
}

/* USER CODE BEGIN Header_MasterSendTaskFun */
/**
* @brief Function implementing the MasterSendTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MasterSendTaskFun */
void MasterSendTaskFun(void *argument)
{
  /* USER CODE BEGIN MasterSendTaskFun */
  uint16_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  /* Infinite loop */
  for(;;)
  {
    eMBMasterReqWriteMultipleHoldingRegister(1, 0, 10, data, 100);
    for (uint16_t i = 0; i < sizeof(data)/sizeof(data[0]); i++)
    {
      data[i] += 1; 
    }
    
    osDelay(1000);
  }
  /* USER CODE END MasterSendTaskFun */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

