/*
 * Serial_DMARx.h
 *
 *  Created on: Feb 27, 2021
 *      Author: naudh
 */

#ifndef SERIAL_DMARX_H_
#define SERIAL_DMARX_H_

#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#include <stdio.h> 	//printf
#include <stdint.h> //privitive type
#include <stdlib.h> //EXIT_SUCCESS EXIT_FAILURE malloc
#include <string.h> //memset

#define DMARX_BUF_MAX		256

typedef struct SerialDMARx_ctx_st {
	UART_HandleTypeDef *huart;
	DMA_HandleTypeDef  *hdma;
	osMessageQId RxQueueHandle;
	osThreadId   RxTaskHandle;
	uint8_t 	 dma_buf[DMARX_BUF_MAX];
} SerialDMARx_ctx_t;


SerialDMARx_ctx_t* 	SerialDMARx_Create();
void 				SerialDMARx_Init(SerialDMARx_ctx_t *ctx, UART_HandleTypeDef *huart, DMA_HandleTypeDef  *hdma);
void 				SerialDMARx_ClearQueue(SerialDMARx_ctx_t *ctx);
int32_t 			SerialDMARx_GetChar(SerialDMARx_ctx_t *ctx, uint8_t *data, uint32_t timeout);



#endif /* SERIAL_DMARX_H_ */
