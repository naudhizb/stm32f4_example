/*
 * mr25h10.h
 *
 *  Created on: Mar 11, 2021
 *      Author: mjlee2
 */

#ifndef MR25H10_H_
#define MR25H10_H_

#include "stm32f4xx_hal.h"
#include <cmsis_os.h>
#include <stdint.h>
#include <stdlib.h>

#define MR25H10_TIMEOUT 	500
#define MR25H10_MAX_SIZE 	0x20000

typedef struct MR25H10_ctx_st {
    SPI_HandleTypeDef *hspi; // Mode 0 or Mode 3. Max 40Mhz.
    GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
    osMutexId MutexHandle;
} MR25H10_ctx_t;

/**
 * @brief MR25H10 status bit
 */
#define STATUS_BIT_SRWD 	7
#define STATUS_BIT_USER3 	6
#define STATUS_BIT_USER2 	5
#define STATUS_BIT_USER1 	4
#define STATUS_BIT_BP1 		3
#define STATUS_BIT_BP0 		2
#define STATUS_BIT_WEL 		1
#define STATUS_BIT_USER0 	0

void 	MR25H10_Init(MR25H10_ctx_t *ctx, SPI_HandleTypeDef *hspi,  GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
int32_t 	MR25H10_Read(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t *pdata, uint16_t size);
int32_t 	MR25H10_Write(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t *pdata, uint16_t size);
int32_t 	MR25H10_Erase(MR25H10_ctx_t *ctx, uint32_t addr, uint32_t size);
uint8_t MR25H10_ReadByte(MR25H10_ctx_t *ctx, uint32_t addr);
void 	MR25H10_WriteByte(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t data);
void    MR25H10_ReadStatus(MR25H10_ctx_t *ctx, uint8_t *status);
void    MR25H10_WriteStatus(MR25H10_ctx_t *ctx, uint8_t *status);



#endif /* MR25H10_H_ */
