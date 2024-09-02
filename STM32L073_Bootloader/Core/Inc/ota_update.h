/*
 * ota_update.h
 *
 *  Created on: Aug 16, 2024
 *      Author: devilalprajapat
 */

#ifndef OTA_UPDATE_H_
#define OTA_UPDATE_H_


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
	OTA_STATE_ABORT
} OTAState_t;

typedef enum{
	ERASE_CONFIG_AREA = 0,
	ERASE_APP_AREA,
	ERASE_DL_AREA,
}OTAErraseEnum_t;

typedef enum OTAErrorCode {
	OTA_OK,
	OTA_ERROR
} OTAErrorCode_t;


typedef enum{
	OTA_PACKET_CMD = 1,
	OTA_PACKET_HEADER,
	OTA_PACKET_DATA,
	OAT_PACKET_RESPONSE
}OTA_PacketType_t;

typedef union mydata {
	uint32_t data;
	uint8_t arr[4];
} mydata_t;

/*
 * Structure to hold OTA Firmware Data and App Firmware Data
 */
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
	uint8_t packet_type;
	uint16_t len;
	uint32_t fw_size;
	uint32_t fw_crc;
}__attribute__((__packed__))OTAHeaderInfo_t;

typedef struct
{
	uint8_t sof;
	uint8_t packet_type;
	uint16_t len;
	uint8_t* buff;
}__attribute__((__packed__))OTA_DataPacket_t;

typedef struct{
	uint8_t sof;
	uint8_t packet_type;
	uint16_t len;
	uint8_t cmd;
	uint32_t crc32;
	uint8_t eof;
}__attribute__((__packed__))OTA_CmdPacket_t;

void Erasse_Flash(uint8_t val);
void write_to_config_flash(ota_config_t *ota_conf);
void write_to_flash(uint32_t addr, uint8_t *buff, uint16_t size);
uint16_t load_new_app(uint32_t addr, uint16_t size);
uint8_t download_and_upgrade_fw(void);
#endif /* OTA_UPDATE_H_ */
