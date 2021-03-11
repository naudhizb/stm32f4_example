
#include "mr25h10.h"



static void MR25H10_ChipSelect(MR25H10_ctx_t *ctx);
static void MR25H10_ChipDeselect(MR25H10_ctx_t *ctx);
static void MR25H10_WriteEnable(MR25H10_ctx_t *ctx);
static void MR25H10_WriteDisable(MR25H10_ctx_t *ctx);

#define	OPCODE_WREN		0x06	/* Write enable */
#define OPCODE_WRDI		0x04	/* Wrtie disable */
#define	OPCODE_RDSR		0x05	/* Read Status Register 1byte */
#define	OPCODE_WRSR		0x01	/* Write Status Register 1byte */
#define	OPCODE_READ		0x03	/* Read Data bytes */
#define	OPCODE_WRITE	0x02	/* Wrtie Data bytes */
#define	OPCODE_SLEEP	0xb9	/* Enter Sleep Mode */
#define	OPCODE_WAKE		0xab	/* Exit Sleep Mode */


/**
 * @brief Initialize MR25H10 Object
 * 
 * @param ctx Context Container pointer
 * @param hspi SPI Handle(SPI shell be Mode 0 or Mode 3. MR25H10 support up to 40Mhz)
 * @param GPIOx Chip Select Port
 * @param GPIO_Pin Chip Select Pin
 */
void MR25H10_Init(MR25H10_ctx_t *ctx, SPI_HandleTypeDef *hspi,  GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	ctx->hspi = hspi;
	ctx->GPIOx = GPIOx;
	ctx->GPIO_Pin = GPIO_Pin;
	osMutexDef(MRAMMutex);
	ctx->MutexHandle = osMutexCreate(osMutex(MRAMMutex));
}

/**
 * @brief Read MRAM
 * 		  (*Note. It can read 0xFFFF in one time -- Half of whole memory)
 * @param ctx MR25H10 Handle
 * @param addr Address [0x0 0x1FFFF]
 * @param pdata pointer of read data
 * @param size size of read data [0x0 0xFFFF]
 */
int32_t MR25H10_Read(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t *pdata, uint16_t size){
	if(MR25H10_MAX_SIZE < (addr + size)){
		return EXIT_FAILURE;
	}
	osMutexWait(ctx->MutexHandle, MR25H10_TIMEOUT);
	uint8_t cmd[4] = {
		OPCODE_READ,
		(uint8_t) (addr >> 16) & 0xFF,
		(uint8_t) (addr >>  8) & 0xFF,
		(uint8_t) (addr >>  0) & 0xFF,
	};

	MR25H10_ChipSelect(ctx);
	HAL_SPI_Transmit(ctx->hspi, cmd, sizeof(cmd), MR25H10_TIMEOUT);
	HAL_SPI_Receive(ctx->hspi, pdata, size, MR25H10_TIMEOUT);
	MR25H10_ChipDeselect(ctx);

	osMutexRelease(ctx->MutexHandle);
	return EXIT_SUCCESS;
}

/**
 * @brief Write Data
 * 		  (*Note. It can read 0xFFFF in one time -- Half of whole memory)
 * @param ctx MR25H10 Handle
 * @param addr Address [0x0 0x1FFFF]
 * @param pdata pointer of write data
 * @param size size of write data [0x0 0xFFFF]
 */
int32_t MR25H10_Write(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t *pdata, uint16_t size){
	if(MR25H10_MAX_SIZE < (addr + size)){
		return EXIT_FAILURE;
	}
	osMutexWait(ctx->MutexHandle, MR25H10_TIMEOUT);
	uint8_t cmd[4] = {
		OPCODE_WRITE,
		(uint8_t) (addr >> 16) & 0xFF,
		(uint8_t) (addr >>  8) & 0xFF,
		(uint8_t) (addr >>  0) & 0xFF,
	};
	MR25H10_WriteEnable(ctx);
	MR25H10_ChipSelect(ctx);
	HAL_SPI_Transmit(ctx->hspi, cmd, sizeof(cmd), MR25H10_TIMEOUT);
	HAL_SPI_Transmit(ctx->hspi, pdata, size, MR25H10_TIMEOUT);
	MR25H10_ChipDeselect(ctx);
	MR25H10_WriteDisable(ctx);
	osMutexRelease(ctx->MutexHandle);
	return EXIT_SUCCESS;
}

/**
 * @brief Erase Data
 * 
 * @param ctx MR25H10 handle
 * @param addr address of erase data
 * @param size size of erase data
 */
int32_t MR25H10_Erase(MR25H10_ctx_t *ctx, uint32_t addr, uint32_t size){
	if(MR25H10_MAX_SIZE < (addr + size)){
		return EXIT_FAILURE;
	}
	osMutexWait(ctx->MutexHandle, MR25H10_TIMEOUT);
	uint8_t cmd[4] = {
		OPCODE_WRITE,
		(uint8_t) (addr >> 16) & 0xFF,
		(uint8_t) (addr >>  8) & 0xFF,
		(uint8_t) (addr >>  0) & 0xFF,
	};
	MR25H10_WriteEnable(ctx);
	MR25H10_ChipSelect(ctx);
	HAL_SPI_Transmit(ctx->hspi, cmd, sizeof(cmd), MR25H10_TIMEOUT);
	for(int32_t i = 0; i < size; i++){
		uint8_t data = 0;
		HAL_SPI_Transmit(ctx->hspi, &data, sizeof(data), MR25H10_TIMEOUT);
	}
	MR25H10_ChipDeselect(ctx);
	MR25H10_WriteDisable(ctx);
	osMutexRelease(ctx->MutexHandle);
	return EXIT_SUCCESS;
}

/**
 * @brief Read one byte from MR25H10
 * 
 * @param ctx MR25H10 handle
 * @param addr read data address
 * @return uint8_t read value
 */
uint8_t MR25H10_ReadByte(MR25H10_ctx_t *ctx, uint32_t addr){
	uint8_t data = 0;
	MR25H10_Read(ctx, addr, &data, sizeof(data));
	return data;
}

/**
 * @brief Write one byte to MR25H10
 * 
 * @param ctx MR25H10 handle
 * @param addr write data address
 * @param data write value
 */
void MR25H10_WriteByte(MR25H10_ctx_t *ctx, uint32_t addr, uint8_t data){
	MR25H10_Write(ctx, addr, &data, sizeof(data));
	return;
}

/**
 * @brief Read MR25H10 status
 * 
 * @param ctx  MR25H10 Handle
 * @param status MR25H10 status.
 */
void MR25H10_ReadStatus(MR25H10_ctx_t *ctx, uint8_t *status){
	uint8_t cmd[2] = {OPCODE_RDSR, 0};
	MR25H10_ChipSelect(ctx);
	HAL_SPI_TransmitReceive(ctx->hspi, cmd, cmd, sizeof(cmd), MR25H10_TIMEOUT);
	MR25H10_ChipDeselect(ctx);
	*status = cmd[1];
}

/**
 * @brief Write MR25H10 status
 * 	(Note. you don't need to use this function unless protect memory area)
 * @param ctx MR25H10 handle
 * @param status MR25H10 status.
 */
void MR25H10_WriteStatus(MR25H10_ctx_t *ctx, uint8_t *status){
	uint8_t cmd[2] = {OPCODE_RDSR, *status};
	MR25H10_WriteEnable(ctx);
	MR25H10_ChipSelect(ctx);
	HAL_SPI_TransmitReceive(ctx->hspi, cmd, cmd, sizeof(cmd), MR25H10_TIMEOUT);
	MR25H10_ChipDeselect(ctx);
	MR25H10_WriteDisable(ctx);
}


static void MR25H10_ChipSelect(MR25H10_ctx_t *ctx){
	HAL_GPIO_WritePin(ctx->GPIOx, ctx->GPIO_Pin, GPIO_PIN_RESET);
}
static void MR25H10_ChipDeselect(MR25H10_ctx_t *ctx){
	HAL_GPIO_WritePin(ctx->GPIOx, ctx->GPIO_Pin, GPIO_PIN_SET);
}
static void MR25H10_WriteEnable(MR25H10_ctx_t *ctx){
	uint8_t cmd = OPCODE_WREN;
	MR25H10_ChipSelect(ctx);
	HAL_SPI_Transmit(ctx->hspi, &cmd, sizeof(cmd), MR25H10_TIMEOUT);
	MR25H10_ChipDeselect(ctx);
}
static void MR25H10_WriteDisable(MR25H10_ctx_t *ctx){
	uint8_t cmd = OPCODE_WRDI;
	MR25H10_ChipSelect(ctx);
	HAL_SPI_Transmit(ctx->hspi, &cmd, sizeof(cmd), MR25H10_TIMEOUT);
	MR25H10_ChipDeselect(ctx);
}
