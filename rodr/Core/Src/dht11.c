#include "dht11.h"
#include "cmsis_os.h"

void DHT11_ReadData(struct DHT11* dht)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	uint16_t Data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	//set pin to output mode
	GPIO_InitStruct.Pin = dht->gpio_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(dht->gpio_port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(dht->gpio_port, dht->gpio_pin, GPIO_PIN_RESET);
	vTaskDelay(pdMS_TO_TICKS(18));

	HAL_GPIO_WritePin(dht->gpio_port, dht->gpio_pin, GPIO_PIN_SET);

	//reset timer and wait 40 us
	__HAL_TIM_SetCounter(dht->htim, 0);
	HAL_TIM_Base_Start(dht->htim);
	while (dht->htim->Instance->CNT < 40);

	//set pin to input mode
	GPIO_InitStruct.Pin = dht->gpio_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(dht->gpio_port, &GPIO_InitStruct);

	//wait for low
	while(!HAL_GPIO_ReadPin(dht->gpio_port, dht->gpio_pin));

	//wait for high
	while(HAL_GPIO_ReadPin(dht->gpio_port, dht->gpio_pin));

	//reset timer
	__HAL_TIM_SetCounter(dht->htim, 0);

	/*read 5 bytes. Every byte represents some type of data:
	 * 1st - integral humidity
	 * 2nd - decimal humidity
	 * 3rd - integral temperature
	 * 4th - decimal temperature
	 * 5th - check-sum
	 */
	for (uint8_t i = 0; i < 5; ++i)
	{
		for (uint8_t j = 0; j < 8; ++j)
		{
			//wait for data bit
			while(!HAL_GPIO_ReadPin(dht->gpio_port, dht->gpio_pin));
			__HAL_TIM_SetCounter(dht->htim, 0);
			while(HAL_GPIO_ReadPin(dht->gpio_port, dht->gpio_pin));

			//if received signal spans 26-28 us then "append" zero else "append" 1
			//made 30 to be sure
			if (dht->htim->Instance->CNT < 30)
				Data[i] <<= 1;
			else
			{
				Data[i] <<= 1;
				Data[i] += 1;
			}

		}
	}

	HAL_TIM_Base_Stop(dht->htim);

	//if data received is incorrect set all to 0 and return
	if(Data[0] + Data[1] + Data[2] + Data[3] != Data[4])
	{
		*(dht->humidity) = 0;
		*(dht->temperature) = 0;
		return;
	}

	*(dht->humidity) = Data[0];
	*(dht->temperature) = Data[2];
}
