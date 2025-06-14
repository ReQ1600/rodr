/*
 * SEN0257.c
 *
 *  Created on: May 26, 2025
 *      Author: Mateusz Drogowski
 */
#include "SEN0257.h"

float SEN0257_readStableAnalog(int samples, ADC_HandleTypeDef *hadc)
{
	float sum = 0;
	for (int i = 0; i < samples; i++)
	{
		HAL_ADC_Start(hadc);
		HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);

		sum += HAL_ADC_GetValue(hadc);
	}
	return sum / samples;
}

float SEN0257_getMedian(float arr[])
{
	float temp[SHORT_WIN];

	for (int i = 0; i < SHORT_WIN; i++) {
		temp[i] = arr[i];
	}

	//sort
	for (int i = 1; i < SHORT_WIN; i++)
	{
		float key = temp[i];
		int j = i - 1;

		while (j >= 0 && temp[j] > key)
		{
			temp[j + 1] = temp[j];
			j--;
		}
		temp[j + 1] = key;
	}

	return temp[SHORT_WIN / 2];
}
