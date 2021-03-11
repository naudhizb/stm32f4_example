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

#define MR25H10_TIMEOUT 	500
#define MR25H10_MAX_SIZE 	0x20000

typedef struct MR25H10_ctx_st {
    SPI_HandleTypeDef *hspi; // Mode 0 or Mode 3. Max 40Mhz.
    GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
    osMutexId MutexHandle;
} MR25H10_ctx_t;

void 	MR25H10_Init(MR25H10_ctx_t *ctx, SPI_HandleTypeDef *hspi,  GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
int32_t MR25H10_Check(MR25H10_ctx_t *ctx);
void 	MR25H10_Read(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t *pdata, uint32_t size);
void 	MR25H10_Write(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t *pdata, uint32_t size);
void 	MR25H10_Erase(MR25H10_ctx_t *ctx, uint32_t addr, uint32_t size);
uint8_t MR25H10_ReadByte(MR25H10_ctx_t *ctx, uint32_t addr);
void 	MR25H10_WriteByte(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t data);



#endif /* MR25H10_H_ */
