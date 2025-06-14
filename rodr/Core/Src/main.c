/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "lwip/api.h"

#include "SEN0257.h"
#include "dht11.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum{
	UP = 0,
	DOWN
} Direction;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_MSG_SIZE 64

//dht
#define DHT11_GPIO_PORT GPIOA
#define DHT11_GPIO_PIN GPIO_PIN_0

//motor
#define PHASE_PORT GPIOC
#define PHASE_PIN GPIO_PIN_2
#define EN_PORT GPIOB
#define EN_PIN GPIO_PIN_1

#define MAX_POS 1220
#define MIN_POS 41
#define POS_TOLERANCE 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

osThreadId defaultTaskHandle;
osThreadId TCPServerTaskHandle;
osThreadId UDPStreamTaskHandle;
osThreadId getPressureTaskHandle;
osThreadId motorTaskHandle;
osMessageQId UDPPressureQueueHandle;
osMessageQId posQueueHandle;
osMessageQId UDPTimestampQueueHandle;
osMutexId dhtMutexHandle;
/* USER CODE BEGIN PV */
uint16_t temperature, humidity;
struct DHT11 dht = {
  DHT11_GPIO_PORT,
  DHT11_GPIO_PIN,
  &temperature,
  &humidity,
  &htim4
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM3_Init(void);
static void MX_UART4_Init(void);
void StartDefaultTask(void const * argument);
void StartTCPServerTask(void const * argument);
void StartUDPStreamTask(void const * argument);
void startGetPressureTask(void const * argument);
void StartMotorTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of dhtMutex */
  osMutexDef(dhtMutex);
  dhtMutexHandle = osMutexCreate(osMutex(dhtMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of UDPPressureQueue */
  osMessageQDef(UDPPressureQueue, 4, float);
  UDPPressureQueueHandle = osMessageCreate(osMessageQ(UDPPressureQueue), NULL);

  /* definition and creation of posQueue */
  osMessageQDef(posQueue, 4, uint16_t);
  posQueueHandle = osMessageCreate(osMessageQ(posQueue), NULL);

  /* definition and creation of UDPTimestampQueue */
  osMessageQDef(UDPTimestampQueue, 4, uint32_t);
  UDPTimestampQueueHandle = osMessageCreate(osMessageQ(UDPTimestampQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of TCPServerTask */
  osThreadDef(TCPServerTask, StartTCPServerTask, osPriorityIdle, 0, 512);
  TCPServerTaskHandle = osThreadCreate(osThread(TCPServerTask), NULL);

  /* definition and creation of UDPStreamTask */
  osThreadDef(UDPStreamTask, StartUDPStreamTask, osPriorityIdle, 0, 256);
  UDPStreamTaskHandle = osThreadCreate(osThread(UDPStreamTask), NULL);

  /* definition and creation of getPressureTask */
  osThreadDef(getPressureTask, startGetPressureTask, osPriorityIdle, 0, 256);
  getPressureTaskHandle = osThreadCreate(osThread(getPressureTask), NULL);

  /* definition and creation of motorTask */
  osThreadDef(motorTask, StartMotorTask, osPriorityIdle, 0, 256);
  motorTaskHandle = osThreadCreate(osThread(motorTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 95;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_HalfDuplex_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|GPIO_PIN_1|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin PB1 LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|GPIO_PIN_1|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN 5 */
  uint32_t time;

  /* Infinite loop */
  for(;;)
  {
	  time = osKernelSysTick();

	  //heartbeat
	  if (time % pdMS_TO_TICKS(200) == 0)
		  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

	  //dht cahnges response every 1 second
	  if (time % pdMS_TO_TICKS(1000) == 0)
	  {
		  if (osMutexWait(dhtMutexHandle, osWaitForever))
		  {
			  DHT11_ReadData(&dht);
			  osMutexRelease(dhtMutexHandle);
		  }
	  }

	  osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTCPServerTask */
/**
* @brief Function implementing the TCPServerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTCPServerTask */
void StartTCPServerTask(void const * argument)
{
  /* USER CODE BEGIN StartTCPServerTask */
	osDelay(100);
	//msg setup
	struct netbuf* pNetbuf;
	void* data;
	uint16_t data_len;
	static char msg[MAX_MSG_SIZE] = { 0 };
	uint16_t msg_len;

	//TCP setup
	struct netconn* pTCP_conn_server;
	pTCP_conn_server = netconn_new(NETCONN_TCP);

	netconn_bind(pTCP_conn_server, IP4_ADDR_ANY, 2000);

	struct netconn* pTCP_conn_client;
	err_t accept_err;

	netconn_listen(pTCP_conn_server);
  /* Infinite loop */
	for(;;)
  {
	  //try to acquire new connection
	  accept_err = netconn_accept(pTCP_conn_server, &pTCP_conn_client);

	  //process conn if acquired
	  if (accept_err == ERR_OK)
	  {
		  while (netconn_recv(pTCP_conn_client, &pNetbuf) == ERR_OK)
		  {
			  do
			  {
				  netbuf_data(pNetbuf, &data, &data_len);

				  //process data
				  if(pNetbuf->p->len >= strlen("PING"))
				  {
					  //handling ping
					  if (!strcmp((char*)pNetbuf->p->payload, "PING"))
						  msg_len = snprintf(msg, MAX_MSG_SIZE, "PONG");

					  //handling setPos
					  else if (!strncmp((char*)pNetbuf->p->payload, "SETPOS", 6))
					  {
						  unsigned int target_pos = 0;

						  if (sscanf((char*)pNetbuf->p->payload + 7, "%u", &target_pos))// +7 so that it skips the "SETPOS:" part
						  {
							  msg_len = snprintf(msg, MAX_MSG_SIZE, "SETPOS_OK");

							  if (target_pos > MAX_POS) target_pos = MAX_POS;
							  osMessagePut(posQueueHandle, target_pos, osWaitForever);
						  }
						  else
							  msg_len = snprintf(msg, MAX_MSG_SIZE, "SETPOS_ERR");
					  }
					  //sends the msg
					  netconn_write(pTCP_conn_client, msg, msg_len, NETCONN_COPY);
				  }
			  }while(netbuf_next(pNetbuf) >= 0);

			  //cleanup
			  netbuf_delete(pNetbuf);
		  }
		  netconn_close(pTCP_conn_client);
		  netconn_delete(pTCP_conn_client);
	  }

    osDelay(100);
  }
  /* USER CODE END StartTCPServerTask */
}

/* USER CODE BEGIN Header_StartUDPStreamTask */
/**
* @brief Function implementing the UDPStreamTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUDPStreamTask */
void StartUDPStreamTask(void const * argument)
{
  /* USER CODE BEGIN StartUDPStreamTask */
	osDelay(100);

	osEvent queue_ret;
	float pressure;
	uint16_t dht_humidity = 0;

	//msg setup
	static char message[MAX_MSG_SIZE];
	uint16_t msg_length = 0;

	struct netbuf* pNetbuf;
	pNetbuf = netbuf_new();

	//UDP setup
	struct netconn* pUDP_conn;
	pUDP_conn = netconn_new(NETCONN_UDP);
	netconn_bind(pUDP_conn, IP4_ADDR_ANY, 1000);

	//ipv4 destination setup
	ip_addr_t dst_addr;
	IP4_ADDR(&dst_addr, 192, 168, 113, 4);

 	/* Infinite loop */
	for(;;)
	{
		//dht
		if (osMutexWait(dhtMutexHandle, 100) == osOK)
		{
			dht_humidity = humidity;
			osMutexRelease(dhtMutexHandle);
		}

		//pressure
		queue_ret = osMessageGet(UDPPressureQueueHandle, osWaitForever);
		if (queue_ret.status == osEventMessage)
			pressure = *(float*)&queue_ret.value.v;

		//timestamp and send
		queue_ret = osMessageGet(UDPTimestampQueueHandle, osWaitForever);
		if (queue_ret.status == osEventMessage)
		{
			msg_length = snprintf(message, MAX_MSG_SIZE, "%lu;%f;%d", queue_ret.value.v, pressure, dht_humidity);
			netbuf_alloc(pNetbuf, msg_length);
			memcpy(pNetbuf->p->payload, message, msg_length);
			netconn_sendto(pUDP_conn, pNetbuf, &dst_addr, 5000);
			netbuf_free(pNetbuf);
		}
		osDelay(10);
	}
  /* USER CODE END StartUDPStreamTask */
}

/* USER CODE BEGIN Header_startGetPressureTask */
/**
* @brief Function implementing the getPressureTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startGetPressureTask */
void startGetPressureTask(void const * argument)
{
  /* USER CODE BEGIN startGetPressureTask */
	osDelay(100);
	uint32_t time = osKernelSysTick();

	float voltage = 0;
	float medianV = 0;

	static float voltage_samples[SHORT_WIN];
	int short_id = 0;
	bool short_filled = false;

	static float long_term_filtered[LONG_WIN];
	int long_id = 0;
	bool long_filled = false;

	static float fir_avg_buff[LONG_WIN] = { 0 };
	int fir_avg_id= 0;
	bool fir_avg_filled = false;

	float long_fir_avg = 0.0f;

	float pressure = 0;

	/* Infinite loop */
	for(;;)
	{
		voltage = SEN0257_readStableAnalog(5, &hadc1) * 5.0f / 4095.0f;

		voltage_samples[short_id] = voltage;
		short_id = GET_AVG_ID(short_id, SHORT_WIN);

		if (short_id == 0) short_filled = true;

		if (short_filled)
		{
			medianV = SEN0257_getMedian(voltage_samples);

			long_term_filtered[long_id] = medianV;
			long_id = GET_AVG_ID(long_id, LONG_WIN);

			if (long_id == 0) long_filled = true;
		}

		fir_avg_buff[fir_avg_id] = medianV;
		fir_avg_id = GET_AVG_ID(fir_avg_id, LONG_WIN);

		if (fir_avg_id == 0) fir_avg_filled = true;

		//calculate only when buffer is full
		if (fir_avg_filled)
		{
			float sum = 0.0f;
			for (int i = 0; i < LONG_WIN-1; i++)
				sum += fir_avg_buff[i];

			long_fir_avg = sum / LONG_WIN;
		}
		else
			long_fir_avg = medianV; // use current val until buff filled

		pressure = (long_fir_avg - OFFSET) * 250.0f;//this is output so it should be queued
		time = osKernelSysTick();

		//packing float into int to put it into queue
		osMessagePut(UDPPressureQueueHandle, *(uint32_t*)&pressure, osWaitForever);
		osMessagePut(UDPTimestampQueueHandle, time, osWaitForever);

		osDelay(20);//should be changed to fit whatever you want to do
	}

  /* USER CODE END startGetPressureTask */
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
* @brief Function implementing the motorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void const * argument)
{
	/* USER CODE BEGIN StartMotorTask */

	//done in phase/enable mode
	int32_t curr_pos = 0;
	uint16_t target = 0;
	Direction last_dir = UP;
	int32_t u = 0;

	osEvent queue_ret;

	//callibration
	HAL_GPIO_WritePin(EN_PORT, EN_PIN , 1);
	HAL_GPIO_WritePin(PHASE_PORT, PHASE_PIN, 1);
	osDelay(5000);
	HAL_GPIO_WritePin(EN_PORT, EN_PIN , 0);
	curr_pos = MIN_POS;

	/* Infinite loop */
	for(;;)
	{
		queue_ret = osMessageGet(posQueueHandle, osWaitForever);
		if (queue_ret.status == osEventMessage)
			target = queue_ret.value.v;

		if (target > MAX_POS) target = MAX_POS;
		else if (target < MIN_POS) target = MIN_POS;

		if (last_dir == UP && curr_pos < target)
		{
			last_dir = DOWN;
			curr_pos -= 40;
		}
		else if(last_dir == DOWN && curr_pos > target)
		{
			last_dir = UP;
			curr_pos += 40;
		}

		while(abs(target - curr_pos) > POS_TOLERANCE)
		{
			if((int16_t)__HAL_TIM_GET_COUNTER(&htim3) < 0)
				__HAL_TIM_SET_COUNTER(&htim3, 0);

			curr_pos = __HAL_TIM_GET_COUNTER(&htim3);
			u = (target - curr_pos);

			//motor control
			HAL_GPIO_WritePin(EN_PORT, EN_PIN , 1);

			if (u > 0) HAL_GPIO_WritePin(PHASE_PORT, PHASE_PIN, 0);

			else HAL_GPIO_WritePin(PHASE_PORT, PHASE_PIN, 1);

			osDelay(10);
		}
		HAL_GPIO_WritePin(EN_PORT, EN_PIN, 0);
	}

	/* USER CODE END StartMotorTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
