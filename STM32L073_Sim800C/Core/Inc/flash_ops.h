/*
 * flash_ops.h
 *
 *  Created on: Aug 16, 2024
 *      Author: devilalprajapat
 */

#ifndef FLASH_OPS_H_
#define FLASH_OPS_H_

#define APP_START_ADDR           0x08008000U
#define DL_SLOT_0_START_ADDR     0x08010000U
#define CONFIG_START_ADDRESS     0x0802FF80U   // last page of sector 47

#define APP_AREA_START_ADDR          (APP_START_ADDR)                                /* Start @ of user Flash area */
#define APP_AREA_END_ADDR            (APP_AREA_START_ADDR + FLASH_PAGE_SIZE * 256)   /* End @ of user Flash area */

#define DL_AREA_START_ADDR           (DL_SLOT_0_START_ADDR)                          /* Start @ of user Flash area */
#define DL_AREA_END_ADDR             (DL_AREA_START_ADDR + FLASH_PAGE_SIZE * 256)   /* End @ of user Flash area */

#define CONFIG_AREA_START_ADDRESS    (CONFIG_START_ADDRESS)

typedef enum {
	OTA_STATE_IDLE,
	OTA_STATE_START,
	OTA_STATE_HEADER,
	OTA_STATE_DATA,
	OTA_STATE_END,
} OTAState_t;

typedef struct ota_config
{
	uint32_t fw_crc;
	uint32_t fw_size;
	uint32_t dl_crc;
	uint32_t dl_size;
	uint8_t flag;
	uint8_t res[3];
}ota_config_t;

typedef struct
{
	uint8_t sof;
	uint16_t len;
	uint32_t fw_size;
	uint32_t fw_crc;
}__attribute__((__packed__))OTAHeaderInfo_t;

typedef enum{
	ERASE_CONFIG_AREA = 0,
	ERASE_APP_AREA,
	ERASE_DL_AREA,
}OTAErraseEnum_t;

typedef enum OTAErrorCode {
	OTA_OK,
	OTA_ERROR
} OTAErrorCode_t;

typedef union mydata {
	uint32_t data;
	uint8_t arr[4];
} mydata_t;

void Erasse_Flash(uint8_t val);
void write_to_config_flash(ota_config_t *ota_conf);
void write_to_flash(uint32_t addr, uint8_t *buff, uint16_t size);

#endif /* FLASH_OPS_H_ */
