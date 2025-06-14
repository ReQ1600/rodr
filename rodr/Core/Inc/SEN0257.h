/*
 * SEN0257.h
 *
 *  Created on: May 26, 2025
 *      Author: Mateusz Drogowski
 */

#ifndef INC_SEN0257_H_
#define INC_SEN0257_H_

#include "stm32f7xx_hal.h"

#include <stdbool.h>

#define OFFSET 0.48f
#define SHORT_WIN 20
#define LONG_WIN 3000
#define FIR_LEN 21

#define GET_AVG_ID(id, win) (id + 1) % win

float SEN0257_readStableAnalog(int samples, ADC_HandleTypeDef *hadc);
float SEN0257_getMedian(float arr[]);

#endif /* INC_SEN0257_H_ */
