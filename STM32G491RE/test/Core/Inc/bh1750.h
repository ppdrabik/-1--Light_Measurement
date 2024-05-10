/*
 * bh1750.h
 *
 *  Created on: Oct 4, 2022
 *      Author: Piotrek
 */

#ifndef INC_BH1750_H_
#define INC_BH1750_H_


#define BH1750_ADRESS	0x23
#define BH1750_I2C_TIMEOUT 10

typedef enum
{
	BH1750_STATE_POWER_DOWN =			0x00,	//0b0000 0000
	BH1750_STATE_POWER_ON =				0x01,	//0b0000 0001
	BH1750_STATE_RESET =				0x07,	//0b0000 0111

}	BH1750_StateTypeDef;


typedef enum
{
	BH1750_MODE_CONST_HIGH_RES =		0x10,	//0b0001 0000
	BH1750_MODE_CONST_HIGH_RES_2 =		0x11,	//0b0001 0001
	BH1750_MODE_CONST_LOW_RES =			0x13,	//0b0001 0111
	BH1750_MODE_ONETIME_HIGH_RES =		0x20,	//0b0010 0000
	BH1750_MODE_ONETIME_HIGH_RES_2 =	0x21,	//0b0010 0001
	BH1750_MODE_ONETIME_LOW_RES =		0x23,	//0b0010 0111

}	BH1750_ModeTypeDef;


void BH1750_SendLux(void);
void BH1750_Mode(BH1750_ModeTypeDef Mode);
void BH1750_Reset(void);
void BH1750_PowerOn(void);


#endif /* INC_BH1750_H_ */
