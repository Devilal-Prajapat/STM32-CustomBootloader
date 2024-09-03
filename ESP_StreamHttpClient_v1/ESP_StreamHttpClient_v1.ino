/**
   StreamHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "ota_update.h"
#define RXD2 16
#define TXD2 17

//#define PRINT_PKT

#define COM_UART  Serial2

#define FW_CHUNK_SIZE     1024

uint8_t FW_BUF[MAX_FW_SIZE] = {0};
uint32_t fw_size = 0;
uint32_t fw_crc = 0;

const char ssid[] = "MyPiXtreme";
const char password[] = "Devd$2101";

uint32_t calculate_crc32(uint8_t *data, uint32_t len) {
  uint32_t POLYNOMIAL = 0x04C11DB7;
  uint32_t crc = 0xFFFFFFFF;
  for (uint32_t i = 0; i < len ; i++)
  {
    crc ^= data[i];
    for (uint8_t j = 0; j < 32; j++) {

      if (crc & 0x80000000)
        crc = (crc << 1 ) ^ POLYNOMIAL;
      else
        crc = crc << 1;
      crc &= 0xFFFFFFFF;
    }
  }
  Serial.printf("crc32: 0x%08x\r\n", crc);
  return (crc);
}

void send_start_packet()
{
  Serial.println("Start Packet ===============================");
  OTA_CmdPacket_t start_packet = {0};
  uint8_t *buf = (uint8_t *)&start_packet;
  
  start_packet.sof = START_OF_FRAME;
  start_packet.packet_type = OTA_PACKET_CMD;
  start_packet.len = CRC_LEN + 1; // Length from data to CRC
  start_packet.cmd = OTA_STATE_START;
  start_packet.crc = calculate_crc32(buf, sizeof(start_packet) - CRC_LEN);
#ifdef PRINT_PKT
  for (uint8_t i = 0; i < sizeof(start_packet); i++)
  {
    printf(" 0x%02x", buf[i]);
  }
#endif
  COM_UART.write(buf, sizeof(start_packet));
}

void send_header_packet(uint32_t fw_size, uint32_t fw_crc)
{
  Serial.println("Header Packet ===============================");
  OTA_HeaderPacket_t header_pkt = {0};
  uint8_t *buf = (uint8_t *)&header_pkt;
  
  header_pkt.sof = START_OF_FRAME;
  header_pkt.packet_type = OTA_PACKET_HEADER;
  header_pkt.len = CRC_LEN + 8; // Length from data to CRC (fw_size, fw_crc, pkt_crc)
  header_pkt.fw_size = fw_size;
  header_pkt.fw_crc = fw_crc;
  header_pkt.crc = calculate_crc32(buf, sizeof(header_pkt) - CRC_LEN);
#ifdef PRINT_PKT
  for (uint8_t i = 0; i < sizeof(header_pkt); i++)
  {
    printf(" 0x%02x", buf[i]);
  }
#endif
  COM_UART.write(buf, sizeof(header_pkt));;
}

void send_abort_packet()
{
  Serial.println("Abort Packet ===============================");
  OTA_CmdPacket_t end_pkt = {0};
  uint8_t *buf = (uint8_t *)&end_pkt;

  end_pkt.sof = START_OF_FRAME;
  end_pkt.packet_type = OTA_PACKET_CMD;
  end_pkt.len = CRC_LEN + 1; // Length from data to CRC
  end_pkt.cmd = OTA_STATE_ABORT;
  end_pkt.crc = calculate_crc32(buf, sizeof(end_pkt) - CRC_LEN);
#ifdef PRINT_PKT
  for (uint8_t i = 0; i < sizeof(end_pkt); i++)
  {
    printf(" 0x%02x", buf[i]);
  }
#endif
  COM_UART.write(buf, sizeof(end_pkt));
}

void send_end_packet()
{
  Serial.println("End Packet ===============================");
  OTA_CmdPacket_t end_pkt = {0};
  uint8_t *buf = (uint8_t *)&end_pkt;

  end_pkt.sof = START_OF_FRAME;
  end_pkt.packet_type = OTA_PACKET_CMD;
  end_pkt.len = CRC_LEN + 1; // Length from data to CRC
  end_pkt.cmd = OTA_STATE_END;
  end_pkt.crc = calculate_crc32(buf, sizeof(end_pkt) - CRC_LEN);
#ifdef PRINT_PKT
  for (uint8_t i = 0; i < sizeof(end_pkt); i++)
  {
    printf(" 0x%02x", buf[i]);
  }
#endif
  COM_UART.write(buf, sizeof(end_pkt));
}

void send_data_packet(uint8_t *buff, uint16_t len)
{
  Serial.println("Data Packet ===============================");
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
#ifdef PRINT_PKT
  for (uint32_t i = 0; i < (idx + CRC_LEN); i++)
  {
    printf(" 0x%02x", pkt[i]);
  }
#endif
  COM_UART.write(pkt, idx + CRC_LEN);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  COM_UART.begin(115200, SERIAL_8N1, RXD2, TXD2);

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("Connecting to Wi-Fi");
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  fw_download();
}
void loop(){
  
}
void fw_download() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    uint32_t sent_bytes = 0;
    Serial.print("[HTTP] begin...\n");

    // configure server and url
    //    http.begin("http://192.168.1.12/ota/STM32L073_Application.bin");
    http.begin("https://stm-ota.s3.amazonaws.com/STM32L073_Application.bin");
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK) {

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        if (len < 0)
        {
          return;
        }
        fw_size = len;

        Serial.printf("[HTTP] file size: %d\n", len);
        // create buffer for read

        uint8_t buff[FW_CHUNK_SIZE] = { 0 };
        uint32_t rd_idx = 0;
        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();
        // read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          // get available data size
          size_t size = stream->available();

          if (size) {
            // read up to (FW_CHUNK_SIZE) byte
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            // write it to Serial

            Serial.println(c, HEX);
            memcpy(&FW_BUF[rd_idx], buff, c);
            if (len > 0) {
              len -= c;
              rd_idx += c;
            }
          }
        }
        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
        Serial.println("Sending fw to mcu");
        /* send your fw here*/
        send_start_packet();
        delay(2000);
        Serial.println("Fimware crc ===============================");
        fw_crc = calculate_crc32(FW_BUF, fw_size);
        send_header_packet(fw_size, fw_crc);
        delay(2000);

        uint32_t app_size = fw_size;
        uint32_t idx = 0;
        while (app_size)
        {
          Serial.printf("Sending [%d/%d]\r\n",(idx /MAX_DATA_SIZE),(fw_size/MAX_DATA_SIZE));
          uint32_t sz =  ((app_size > MAX_DATA_SIZE) ? MAX_DATA_SIZE : app_size);
          send_data_packet(&FW_BUF[idx], sz);
          idx += sz;
          app_size -= sz;          
          delay(1000);
        }
        if (idx == fw_size)
        {
          send_end_packet();
        } else
        {
          send_abort_packet();
        }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  delay(10000);
}
