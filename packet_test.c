#include <stdio.h>
#include <string.h>
#include "ota_update.h"

uint32_t calculate_crc32(uint8_t *data,uint32_t len){
    uint32_t POLYNOMIAL = 0x04C11DB7;
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i< len ; i++)
    {
        crc ^= data[i];
        for(uint8_t j = 0; j<32; j++){
 
            if(crc & 0x80000000)
                crc = (crc <<1 ) ^ POLYNOMIAL;
            else
                crc = crc << 1;
            crc &= 0xFFFFFFFF;
        }
    }
    printf("\r\ncrc32: 0x%08x \r\n",crc);
    return (crc);
}

void send_start_packet()
{
  OTA_CmdPacket_t start_packet = {0};
  uint8_t *buf = (uint8_t *)&start_packet;
  
  start_packet.sof = START_OF_FRAME;
  start_packet.packet_type = OTA_PACKET_CMD;
  start_packet.len = CRC_LEN + 1; // Length from data to CRC
  start_packet.cmd = OTA_STATE_START;
  start_packet.crc = calculate_crc32(buf, sizeof(start_packet) - CRC_LEN);
  for(uint8_t i = 0; i< sizeof(start_packet); i++)
  {
    printf(" 0x%02x", buf[i]);
  }
//   COM_UART.write(buf, sizeof(start_packet));
}

void send_header_packet(uint32_t fw_size)
{
  OTA_HeaderPacket_t header_pkt = {0};
  uint8_t *buf = (uint8_t *)&header_pkt;
  header_pkt.sof = START_OF_FRAME;
  header_pkt.packet_type = OTA_PACKET_HEADER;
  header_pkt.len = CRC_LEN + 8; // Length from data to CRC (fw_size, fw_crc, pkt_crc)
  header_pkt.fw_size = fw_size;
  header_pkt.fw_crc = 0;
  header_pkt.crc = calculate_crc32(buf, sizeof(header_pkt) - CRC_LEN);
  for(uint8_t i = 0; i< sizeof(header_pkt); i++)
  {
    printf(" 0x%02x", buf[i]);
  }
//   COM_UART.write(buf, sizeof(header_pkt));;
}

void send_end_packet()
{
  OTA_CmdPacket_t end_pkt = {0};
  uint8_t *buf = (uint8_t *)&end_pkt;
  
  end_pkt.sof = START_OF_FRAME;
  end_pkt.packet_type = OTA_PACKET_CMD;
  end_pkt.len = CRC_LEN + 1; // Length from data to CRC
  end_pkt.cmd = OTA_STATE_END;
  end_pkt.crc = calculate_crc32(buf, sizeof(end_pkt) - CRC_LEN);
  for(uint8_t i = 0; i< sizeof(end_pkt); i++)
  {
    printf(" 0x%02x", buf[i]);
  }
//   COM_UART.write(buf, sizeof(end_pkt));
}

void send_data_packet(uint8_t *buff, uint16_t len)
{
  uint32_t sz = (len + CRC_LEN);
  uint8_t pkt[MAX_PACKET_SIZE];
  uint32_t idx = 0;
  pkt[idx++] = START_OF_FRAME;
  pkt[idx++] = OTA_PACKET_DATA;
  pkt[idx++] = ((sz >> 0) & 0xFF);  // lsb
  pkt[idx++] = ((sz >> 8) & 0xFF);
  // printf("\r\n idx : %d\r\n",idx);
  memcpy(&pkt[idx], buff, len);
  idx = idx + len;  
  // printf("\r\n idx : %d\r\n",idx);
  uint32_t crc = calculate_crc32(pkt, idx);
  memcpy(&pkt[idx], (char *)&crc, CRC_LEN); 
  // printf("\r\n idx : %d\r\n",idx + CRC_LEN);
  for(uint32_t i = 0; i< (idx + CRC_LEN); i++)
  {
    printf(" 0x%02x", pkt[i]);
  }
  // COM_UART.write(packet, idx + CRC_LEN);
}
int main()
{
    printf("Hello World\r\n");
    send_start_packet();
    send_header_packet(1024);
    send_end_packet();
    char buf[MAX_DATA_SIZE] = {0};
    for(uint32_t i = 0; i< MAX_DATA_SIZE; i++)
    {
        buf[i] = i;
    }
    send_data_packet(buf, MAX_DATA_SIZE);
    return 0;
}
