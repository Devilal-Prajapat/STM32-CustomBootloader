/*
 * ota_process.c
 *
 *  Created on: Aug 1, 2024
 *      Author: devilalprajapat
 */

#include "main.h"
#include "debug.h"
#include <string.h>
#include <ota_update.h>
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Address = 0, PAGEError = 0;
__IO uint32_t data32 = 0, MemoryProgramStatus = 0;

/*Variable used for Erase procedure*/

/*
 typedef enum{
 ERASE_CONFIG_AREA = 0,
 ERASE_APP_AREA,
 ERASE_DL_AREA,
 }OTAErraseEnum_t;

 typedef enum OTAErrorCode {
 OTA_OK,
 OTA_ERROR
 } OTAErrorCode_t;
 */

const uint16_t MAX_PACKET_LEN = 264;
const uint16_t OTA_PACKET_DATA_LEN = 256;

uint8_t rx_buff[512] = { 0 };
static char *TAG = "OTA";

#define START_OF_FRAME   0xAA
#define END_OF_FRAME     0xBB

extern ota_config_t *ota_config;
OTAState_t ota_state = OTA_STATE_IDLE;

uint16_t receive_chunk(void);

void Erasse_Flash(uint8_t val) {
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef EraseInitStruct = { 0 };
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

uint16_t load_new_app(uint32_t addr, uint16_t size) {

	DEBUG_LOG(TAG, "Erasing App Area");
	Erasse_Flash(ERASE_APP_AREA);
	HAL_Delay(2000);

	HAL_FLASH_Unlock();
	uint32_t address = APP_AREA_START_ADDR;
	/* Program the user Flash area word by word
	 (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
	DEBUG_LOG(TAG, "loading new app");
	uint32_t *ptr = (uint32_t*) DL_AREA_START_ADDR;

	for (uint16_t i = 0; i < size; i += 4) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *ptr)
				== HAL_OK) {
			address = address + 4;
			ptr++;
		} else {
			DEBUG_LOG(TAG, "Error in loading App");
		}
	}
	HAL_FLASH_Lock();
	return 0;
}

uint8_t download_and_upgrade_fw(void) {
	static uint32_t addr = DL_AREA_START_ADDR;
	static uint32_t app_size = 0;
	uint32_t fw_size = 0;
	uint32_t fw_crc = 0;
	ota_state = OTA_STATE_START;
	uint16_t len = 0;
	OTAErrorCode_t ret = OTA_ERROR;
	uint8_t debug_buf[64];
	HAL_UART_Transmit(COM_UART, (uint8_t*) "OTA", 3, 100);
	while (ota_state != OTA_STATE_IDLE) {
		len = receive_chunk();
		if (len != 0) {
			/* check for abort packet */
			OTA_CmdPacket_t *abort_packet = (OTA_CmdPacket_t*) rx_buff;
			if (abort_packet->packet_type == OTA_PACKET_CMD &&
					abort_packet->cmd == OTA_STATE_ABORT) {
				DEBUG_LOG(TAG, "abort  ota process");
				ota_state = OTA_STATE_IDLE;
				ret =  OTA_ERROR;
				break;
			}

			switch (ota_state) {
			case OTA_STATE_IDLE:
				DEBUG_LOG(TAG, "OTA_STATE_IDLE")
				break;
			case OTA_STATE_START:
				OTA_CmdPacket_t *start_packet = (OTA_CmdPacket_t*) rx_buff;
				if (start_packet->packet_type == OTA_PACKET_CMD) {
					if (start_packet->cmd == OTA_STATE_START) {
						DEBUG_LOG(TAG, "START");
						ota_state = OTA_STATE_HEADER;
					}
				}
				break;
			case OTA_STATE_HEADER:
				OTAHeaderInfo_t *ota_header = (OTAHeaderInfo_t*) rx_buff;
				if (start_packet->packet_type == OTA_PACKET_HEADER) {
					fw_size = ota_header->fw_size;
					fw_crc = ota_header->fw_crc;
					addr = DL_AREA_START_ADDR;
					app_size = 0;
					Erasse_Flash(ERASE_DL_AREA);
					ota_state = OTA_STATE_DATA;
				}
				break;

			case OTA_STATE_DATA:
				OTA_DataPacket_t *data = (OTA_DataPacket_t *)rx_buff;
				if(data->packet_type == OTA_PACKET_DATA){
					DEBUG_LOG(TAG, "Packet Write");
					write_to_flash(addr, &rx_buff[4], (len - 8)); // <4 byte crc and 4 byte sof & packet type, len>
					addr += (len - 8);
					app_size += (len - 8);
					if (len != MAX_PACKET_LEN) {
						ota_state = OTA_STATE_END;
					}

					sprintf(debug_buf, "FW Received No Of Packet : [%ld/%ld]", (app_size/OTA_PACKET_DATA_LEN), (fw_size/OTA_PACKET_DATA_LEN));
					DEBUG_LOG(TAG, debug_buf)
					;
					HAL_UART_Transmit(COM_UART, (uint8_t*) "ack", 3, 100);
				}
				break;
			case OTA_STATE_END:
				OTA_CmdPacket_t *end_packet = (OTA_CmdPacket_t*) rx_buff;
				if (end_packet->packet_type == OTA_PACKET_CMD) {
					if (end_packet->cmd == OTA_STATE_END) {
						DEBUG_LOG(TAG, "END");
						sprintf((char*) debug_buf,
								"Received size = %ld, fw_size = %ld", app_size,
								fw_size);
						DEBUG_LOG(TAG, debug_buf);
						if (app_size != fw_size) {
							DEBUG_LOG(TAG, "Received FW size mismatch!!!");
							ota_state = OTA_STATE_IDLE;
							ret = OTA_ERROR;
						} else {
							DEBUG_LOG(TAG, "Received FW size match");
							uint32_t *ptr = (uint32_t*) DL_AREA_START_ADDR;
							uint32_t crc = calculate_crc32(ptr, fw_size);
							if (crc == fw_crc) {
								DEBUG_LOG(TAG, "DL FW CRC OK");
								ota_config_t ota_config_temp = { };
								memcpy(&ota_config_temp, ota_config,
										sizeof(ota_config_t));
								Erasse_Flash(ERASE_CONFIG_AREA);
								ota_config_temp.flag = 1;
								ota_config_temp.dl_size = app_size;
								ota_config_temp.dl_crc = crc;
								write_to_config_flash(&ota_config_temp);
								ota_state = OTA_STATE_IDLE;
								ret = OTA_OK;
							} else {
								DEBUG_LOG(TAG, "FW CRC Mismatch");
								ota_state = OTA_STATE_IDLE;
								ret = OTA_ERROR;
							}
						}
					}
				}
				break;
			default:
				break;
			}

#if 0
			if (strstr((char*) &rx_buff[3], "START")) {
				DEBUG_LOG(TAG, "START");
				addr = DL_AREA_START_ADDR;
				app_size = 0;
				Erasse_Flash(ERASE_DL_AREA);
			} else if (strstr((char*) &rx_buff[3], "END")) {
				DEBUG_LOG(TAG, "END");
				ota_config_t ota_config_temp = { };
				memcpy(&ota_config_temp, ota_config, sizeof(ota_config_t));
				Erasse_Flash(ERASE_CONFIG_AREA);
				ota_config_temp.flag = 1;
				ota_config_temp.len = app_size;
				write_to_config_flash(&ota_config_temp);
				break;
			} else {
				DEBUG_LOG(TAG, "PACKET Write");
				write_to_flash(addr, &rx_buff[3], (len - 3));
				addr += (len - 3);
				app_size += (len - 3);
			}
			DEBUG_LOG(TAG, rx_buff);
#endif
		}
//		else
//		{
//			HAL_UART_AbortReceive(COM_UART);
//		}
	}
	return ret;
}

uint16_t receive_chunk() {
	/*
	 | START | packet_type |LEN | DATA | CRC
	 */
	char debug_buf[256];
	uint8_t ret = 0;
	uint16_t data_len = 0;
	uint16_t index = 0;
	memset(rx_buff, 0, 512);
//	__HAL_UART_FLUSH_DRREGISTER(COM_UART);
	do {
		// receive start byte
		ret = HAL_UART_Receive(COM_UART, &rx_buff[index++], 1,
		HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			break;
		}
		if (rx_buff[0] != START_OF_FRAME) {
			ret = OTA_ERROR;
			break;
		}

		// <Packet Type>
		ret = HAL_UART_Receive(COM_UART, &rx_buff[index++], 1,
		HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			ret = OTA_ERROR;
			break;
		}

		// Receive len <include data + crc+ EOF>
		ret = HAL_UART_Receive(COM_UART, &rx_buff[index], 2, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			ret = OTA_ERROR;
			break;
		}
		// First LSB then MSB
		data_len = ((rx_buff[index])) | ((rx_buff[index + 1]) << 8);
		index += 2;

		ret = HAL_UART_Receive(COM_UART, &rx_buff[index], data_len,
		HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			ret = OTA_ERROR;
			break;
		}
		index += data_len;
		// check crc
		uint32_t crc_pos = 4 + data_len - 4;
		uint32_t crc = ((rx_buff[crc_pos])) | ((rx_buff[crc_pos + 1]) << 8)
				| ((rx_buff[crc_pos + 2]) << 16)
				| ((rx_buff[crc_pos + 3]) << 24);

		uint32_t cal_crc = calculate_crc32(rx_buff, 4 + data_len - 4);

		if (crc != cal_crc) {
			DEBUG_LOG(TAG, "crc error in packet");
		}

	} while (0);
	if (ret != HAL_OK) {
		//clear the index if error
		index = 0u;
	}

	if (ret != OTA_OK) {
		//clear the index if error
		index = 0u;
	}

	if (MAX_PACKET_LEN < index) {
		sprintf(debug_buf, "Received more Expected = %d, Received = %d\r\n",
				MAX_PACKET_LEN, index);
		DEBUG_LOG(TAG, debug_buf);
		index = 0u;
	}else
	{
		sprintf(debug_buf, "Received Expected = %d, Received = %d\r\n",
				MAX_PACKET_LEN, index);
		DEBUG_LOG(TAG, debug_buf);
	}
//	if(index == 0)
//	{
//		HAL_UART_AbortReceive(COM_UART);
//	}
	return index;

}
