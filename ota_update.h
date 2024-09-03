
#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <stdint.h>

#define START_OF_FRAME   0xAA
#define END_OF_FRAME     0xBB
#define CRC_LEN          0x04
#define HEADER_LEN       0x08
#define MAX_DATA_SIZE    256U
#define MAX_PACKET_SIZE  ((MAX_DATA_SIZE) + (HEADER_LEN))

typedef enum
{
  OTA_STATE_IDLE = 0x00,
  OTA_STATE_START,
  OTA_STATE_HEADER,
  OTA_STATE_DATA,
  OTA_STATE_END,
  OTA_STATE_ABORT
} OTAState_t;

typedef enum{
  OTA_PACKET_CMD = 0x01,
  OTA_PACKET_HEADER,
  OTA_PACKET_DATA,
  OTA_PACKET_RESPONSE
}OTA_PacketType_t;

typedef struct{
  uint8_t sof;
  uint8_t packet_type;
  uint16_t len;
  uint8_t cmd;
  uint32_t crc;
//  uint8_t eof;
}__attribute__((__packed__))OTA_CmdPacket_t;;

typedef struct{
  uint8_t sof;
  uint8_t packet_type;
  uint16_t len;
  uint32_t fw_size;
  uint32_t fw_crc;
  uint32_t crc;
//  uint8_t eof;
  
}__attribute__((__packed__))OTA_HeaderPacket_t;


typedef struct{
  uint8_t sof;
  uint8_t packet_type;
  uint16_t len;
  uint8_t *buff;
}OTA_DataPacket_t;

#endif /* OTA_UPDATE_H */
