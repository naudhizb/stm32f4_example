/*
 * Serial_DMARx.c
 *
 *  Created on: Feb 26, 2021
 *      Author: naudh
 */

#include "Serial_DMARx.h"

static void SerialDMARx_Task(SerialDMARx_ctx_t *ctx);

SerialDMARx_ctx_t* SerialDMARx_Create(){
	SerialDMARx_ctx_t* ret = malloc(sizeof(SerialDMARx_ctx_t));
	return ret;
}

void SerialDMARx_Init(SerialDMARx_ctx_t *ctx, UART_HandleTypeDef *huart, DMA_HandleTypeDef  *hdma){

	memset(ctx, 0, sizeof(SerialDMARx_ctx_t));

	ctx->huart = huart;
	ctx->hdma = hdma;

	osMessageQDef(RXQ, DMARX_BUF_MAX, uint8_t);
	ctx->RxQueueHandle = osMessageCreate(osMessageQ(RXQ), NULL);
	osThreadDef(RxTask, (os_pthread)SerialDMARx_Task, osPriorityHigh, 0, 256);
	ctx->RxTaskHandle = osThreadCreate(osThread(RxTask), ctx);
}

void SerialDMARx_ClearQueue(SerialDMARx_ctx_t *ctx){
	osEvent evt;
	do {
		evt = osMessageGet(ctx->RxQueueHandle, 0);
		if(evt.status == osEventMessage){
			printf("Clear Q: 0x%02X\r\n", (unsigned int)evt.value.v);
		}
	} while(evt.status == osEventMessage);
}

int32_t SerialDMARx_GetChar(SerialDMARx_ctx_t *ctx, uint8_t *data, uint32_t timeout){
	osEvent evt = osMessageGet(ctx->RxQueueHandle, timeout);
	if(evt.status == osEventMessage){
		*data = evt.value.v;
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

int32_t SerialDMARx_ReceivePacket(SerialDMARx_ctx_t *ctx, uint8_t *buf, uint32_t timeout){
	int32_t len = 0;
	osEvent evt;
	const uint32_t silent_interval = 3;

	evt = osMessageGet(ctx->RxQueueHandle, timeout);
	if(evt.status == osEventMessage){
		buf[len++] = evt.value.v;
		// fall-through
	} else { // timeout
		len = 0;
		return len;
	}

	do {
		evt = osMessageGet(ctx->RxQueueHandle, silent_interval);
		if(evt.status == osEventMessage){
			buf[len++] = evt.value.v;
			if(DMARX_BUF_MAX <= len){ // saturate
				len = DMARX_BUF_MAX-1;
			}
		}
	} while(evt.status == osEventMessage);

	return len;
}

static void SerialDMARx_Task(SerialDMARx_ctx_t *ctx){
	const int32_t dma_size = DMARX_BUF_MAX;
	HAL_StatusTypeDef status = HAL_OK;
	UART_HandleTypeDef *huart = ctx->huart;
	DMA_HandleTypeDef  *hdma = ctx->hdma;
	uint32_t curr_pos = 0;
	uint32_t prev_pos = 0;
	uint32_t PreviousWakeTime = osKernelSysTick();
	uint8_t *pbuf = ctx->dma_buf;
	memset(ctx->dma_buf, 0, sizeof(ctx->dma_buf));

	do{
		status = HAL_UART_Receive_DMA(huart, pbuf, dma_size);
	} while(status != HAL_OK);

	while(1){
		PreviousWakeTime = osKernelSysTick();

		curr_pos = dma_size - hdma->Instance->NDTR; // [0 255]
		while(curr_pos != prev_pos){
			osMessagePut(ctx->RxQueueHandle, ctx->dma_buf[prev_pos], 1000);
			prev_pos++;
			if(prev_pos == dma_size){
				prev_pos = 0;
			}
		}

		// Restore Routine
		HAL_DMA_StateTypeDef dma_status = HAL_DMA_GetState(hdma);
		if(HAL_DMA_STATE_READY == dma_status){
			// DMA Aborted(because of error). try to restore
				status = HAL_UART_Receive_DMA(huart, pbuf, dma_size);
				curr_pos = 0;
				prev_pos = 0;
		}

		osDelayUntil(&PreviousWakeTime, 1);
	}
}
