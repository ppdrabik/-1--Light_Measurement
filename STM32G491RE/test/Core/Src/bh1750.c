/*
 * bh1750.c
 *
 *  Created on: Oct 4, 2022
 *      Author: Piotrek
 *
 */
#include "stdio.h"
#include "stdint.h"
#include "i2c.h"
#include "bh1750.h"
#include "usart.h"
#include <string.h>

BH1750_StateTypeDef bh1750_state;

void BH1750_SendLux(void)
{
	uint8_t value[2];
	HAL_I2C_Master_Receive(&hi2c2, (BH1750_ADRESS<<1), value, 2, BH1750_I2C_TIMEOUT);
	HAL_UART_Transmit(&huart4, value, 2, 30);
}


void BH1750_Mode(BH1750_ModeTypeDef mode)
{
	HAL_I2C_Master_Transmit(&hi2c2,(BH1750_ADRESS<<1),&mode, 1, BH1750_I2C_TIMEOUT);
}


void BH1750_Reset(void)
{
	bh1750_state = BH1750_STATE_RESET;
	HAL_I2C_Master_Transmit(&hi2c2,(BH1750_ADRESS<<1),&bh1750_state, 1, BH1750_I2C_TIMEOUT);
}

void BH1750_PowerOn(void)
{
	bh1750_state = BH1750_STATE_POWER_ON;
	HAL_I2C_Master_Transmit(&hi2c2,(BH1750_ADRESS<<1),&bh1750_state, 1, BH1750_I2C_TIMEOUT);
}
