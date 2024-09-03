/*
 * flash_ops.c
 *
 *  Created on: Aug 16, 2024
 *      Author: devilalprajapat
 */
#include "main.h"
#include "debug.h"
#include "flash_ops.h"

uint32_t Address = 0, PAGEError = 0;
__IO uint32_t data32 = 0, MemoryProgramStatus = 0;
static const char *TAG = "FLASH";

void Erasse_Flash(uint8_t val) {
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef EraseInitStruct = {0};
	/* Erase the user Flash area
	 (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
	if (val == ERASE_APP_AREA) {

		/* Fill EraseInit structure*/
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.PageAddress = APP_AREA_START_ADDR;
		EraseInitStruct.NbPages = (APP_AREA_END_ADDR - APP_AREA_START_ADDR)
				/ FLASH_PAGE_SIZE;
	} else if (val == ERASE_DL_AREA) {

		/* Fill EraseInit structure*/
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.PageAddress = DL_AREA_START_ADDR;
		EraseInitStruct.NbPages = (DL_AREA_END_ADDR - DL_AREA_START_ADDR)
				/ FLASH_PAGE_SIZE;
	} else {
		/* Erase the config Flash area
		 (area defined by CONFIG_START_ADDRESS ) ***********/
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.PageAddress = CONFIG_AREA_START_ADDRESS;
		EraseInitStruct.NbPages = 1;

	}
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
		/*
		 Error occurred while page erase.
		 User can add here some code to deal with this error.
		 PAGEError will contain the faulty page and then to know the code error on this page,
		 user can call function 'HAL_FLASH_GetError()'
		 */
		/* Infinite loop */
		while (1) {
			/* Make LED2 blink (100ms on, 2s off) to indicate error in Erase operation */
			DEBUG_LOG(TAG, "Error in Erase");
		}
	}
	HAL_FLASH_Unlock();
}

void write_to_config_flash(ota_config_t *ota_conf) {
	HAL_FLASH_Unlock();
	uint32_t address = CONFIG_AREA_START_ADDRESS;
	mydata_t d = { 0 };
	uint16_t size = sizeof(ota_config_t);
	uint16_t len = size / 4;
	uint16_t rem = size % 4;
	if (rem != 0) {
		len++;
	}
	/* Program the user Flash area word by word
	 (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
	DEBUG_LOG(TAG, "Config Write Start");
	uint32_t *data = (uint32_t*) ota_conf;
	for (uint16_t i = 0; i < len; i++) {
		d.data = *data;
//		uint32_t  data = ((buff[index + 0] << 0) | (buff[index + 1] << 8)  | (buff[index + 2] << 16) | (buff[index + 3] << 24));

		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, d.data)
				== HAL_OK) {
			address = address + 4;
			data++;
		} else {
			DEBUG_LOG(TAG, "Error Config Write");
		}
	}
	HAL_FLASH_Lock();
}

void write_to_flash(uint32_t addr, uint8_t *buff, uint16_t size) {
	HAL_FLASH_Unlock();
	uint32_t address = addr;
	mydata_t d = { 0 };
	/* Program the user Flash area word by word
	 (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
	DEBUG_LOG(TAG, "DL Flash Write Start");
	for (uint16_t i = 0; i < size; i += 4) {
//
		d.arr[0] = buff[i];
		d.arr[1] = buff[i + 1];
		d.arr[2] = buff[i + 2];
		d.arr[3] = buff[i + 3];

//			uint16_t index = i;
//			uint32_t  data = ((buff[index + 0] << 0) | (buff[index + 1] << 8)  | (buff[index + 2] << 16) | (buff[index + 3] << 24));

		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, d.data)
				== HAL_OK) {
			address = address + 4;
		} else {
			DEBUG_LOG(TAG, "Error DL Flash WRITE");
		}
	}
	HAL_FLASH_Lock();
}

