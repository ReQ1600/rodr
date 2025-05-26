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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_MSG_SIZE 64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

osThreadId defaultTaskHandle;
osThreadId TCPServerTaskHandle;
osThreadId UDPStreamTaskHandle;
osThreadId getPressureTaskHandle;
osMessageQId pressureQueueHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_ADC1_Init(void);
void StartDefaultTask(void const * argument);
void StartTCPServerTask(void const * argument);
void StartUDPStreamTask(void const * argument);
void startGetPressureTask(void const * argument);

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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

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
  /* definition and creation of pressureQueue */
  osMessageQDef(pressureQueue, 2, float);
  pressureQueueHandle = osMessageCreate(osMessageQ(pressureQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of TCPServerTask */
  osThreadDef(TCPServerTask, StartTCPServerTask, osPriorityIdle, 0, 512);
  TCPServerTaskHandle = osThreadCreate(osThread(TCPServerTask), NULL);

  /* definition and creation of UDPStreamTask */
  osThreadDef(UDPStreamTask, StartUDPStreamTask, osPriorityIdle, 0, 516);
  UDPStreamTaskHandle = osThreadCreate(osThread(UDPStreamTask), NULL);

  /* definition and creation of getPressureTask */
  osThreadDef(getPressureTask, startGetPressureTask, osPriorityIdle, 0, 128);
  getPressureTaskHandle = osThreadCreate(osThread(getPressureTask), NULL);

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
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
  int ctr = 0;
  float sensor_data = 2137.223f;
  /* Infinite loop */
  for(;;)
  {
	  if (ctr % 200 == 0) HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

	  if (ctr % 100 == 0)
	  {
		  //mayne free mutex that allows for udp sending here??
	  }

	  ++ctr;
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
	char msg[MAX_MSG_SIZE] = { 0 };
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
					  msg_len = 0;
					  //handling ping
					  if (!strcmp((char*)pNetbuf->p->payload, "PING"))
						  msg_len = snprintf(msg, MAX_MSG_SIZE, "PONG");

					  //handling setPos
					  else if (!strncmp((char*)pNetbuf->p->payload, "SETPOS", 6))
					  {
						  float target_pos = 0;

						  if (sscanf((char*)pNetbuf->p->payload + 7, "%f", &target_pos))// +7 so that it skips the "SETPOS:" part
						  {
							  msg_len = snprintf(msg, MAX_MSG_SIZE, "posOK");
							  //TODO: implement setpos
						  }
						  else
							  msg_len = snprintf(msg, MAX_MSG_SIZE, "posERR");
					  }
					  //sends the msg
					  if (msg_len)
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

	//msg setup
	char message[MAX_MSG_SIZE];
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
//		queue_ret = osMessageGet(UDPMsgQueueHandle, osWaitForever);

		if (queue_ret.status == osEventMessage)
		{
			float data;
			//unpacking data from queue
			memcpy(&data, &queue_ret.value.v, sizeof(float));

			msg_length = snprintf(message, MAX_MSG_SIZE, "0;%f;0;", data);
			netbuf_alloc(pNetbuf, msg_length);
			memcpy(pNetbuf->p->payload, message, msg_length);
			netconn_sendto(pUDP_conn, pNetbuf, &dst_addr, 5000);
			netbuf_free(pNetbuf);

			osDelay(10);
		}

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
	float voltage = 0;
	float medianV = 0;

	float voltage_samples[SHORT_WIN];
	int short_id = 0;
	bool short_filed = false;

	float long_term_filtered[LONG_WIN];
	int long_id = 0;
	bool long_filled = false;

	float fir_avg_buff[LONG_WIN] = { 0 };
	int fir_avg_id= 0;
	bool fir_avg_filled = false;

	float long_fir_avg = 0.0f;
	float long_fir = 0.0f;

	float pressure = 0;
  /* Infinite loop */
	for(;;)
	{
		voltage = SEN0257_readStableAnalog(5, &hadc1) * 5.0f / 4095.0f;

		voltage_samples[short_id] = voltage;
		short_id = GET_AVG_ID(short_id, SHORT_WIN);

		if (short_id == 0) short_filed = true;


		if (short_filed)
		{
			medianV = SEN0257_getMedian(voltage_samples, SHORT_WIN);

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
			for (int i = 0; i < LONG_WIN; i++)
				sum += fir_avg_buff[i];

			long_fir_avg = sum / LONG_WIN;
		}
		else
			long_fir_avg = medianV;  // use current val until buff filled

		pressure = (long_fir_avg - OFFSET) * 250.0f;//this is output so it should be queued

		//packing float into int to put it into queue
		uint32_t packed_data;
		memcpy(&packed_data, &pressure, sizeof(float));

		osMessagePut(pressureQueueHandle, packed_data, osWaitForever);

		osDelay(10);//should be changed to fit whatever we want to do
	}
  /* USER CODE END startGetPressureTask */
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
