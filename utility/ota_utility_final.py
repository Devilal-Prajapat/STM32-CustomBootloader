from serial import Serial
import struct
from time import sleep
import os
import sys

COM_PORT = "COM10"
BAUDRATE = 115200
file_name = "STM32L073_Application.bin"

SOF = 0xAA
EOF = 0xBB
CRC_LEN = 0x04
EOF_LEN = 0x01

class OTA_PACKET_Type:
    OTA_PACKET_CMD      = 0x01
    OTA_PACKET_HEADER   = 0x02
    OTA_PACKET_DATA     = 0x03
    OTA_PACKET_RESPONSE = 0x04

class OTA_STATE_Type:
    OTA_STATE_IDLE   = 0x00
    OTA_STATE_START  = 0x01
    OTA_STATE_HEADER = 0x02
    OTA_STATE_DATA   = 0x03
    OTA_STATE_END    = 0x04
    OTA_STATE_ABORT  = 0x05

""" ================ State variable for OTA Process ================ """
state  = OTA_STATE_Type.OTA_STATE_IDLE

""" ================Serial port ================ """
ser = Serial(port=COM_PORT,  
            baudrate=BAUDRATE,
            stopbits=1,
            timeout=1)

def open_com():
    try:
        if not(ser.is_open):
            ser.open()
    except Exception as e:
        print(f"error in open {e}")

def close_com():
    try:
        ser.close()        
    except Exception as e:
        print(f"error in open {e}")

if not(ser.is_open):
    ser.open()

""" ================ Caculate CRC32 ================ """
def calculate_crc32(data, len):
    POLYNOMIAL = 0x04C11DB7
    crc = 0xFFFFFFFF
    data = bytearray(data)
    for b in data:
        crc ^= b
        for _ in range(0, 32):
            if(crc & 0x80000000):
                crc = (crc <<1 ) ^ POLYNOMIAL
            else:
                crc = crc << 1
            crc &= 0xFFFFFFFF
    print(f"crc32: {hex(crc)}")
    return (crc)

# def calculate_crc32(data, len):
#     POLYNOMIAL = 0x04C11DB7
#     crc = 0xFFFFFFFF
#     data = bytearray(data)
#     for b in data:
#         crc ^= b
#         for _ in range(0, 32):
#             if(crc & 0x01):
#                 crc = (crc >>1 ) ^ POLYNOMIAL
#             else:
#                 crc = crc >> 1
#     print(f"crc32: {hex(crc)}")
#     return hex(crc)

""" ================ Calculate FW CRC ================ """
def get_fw_crc():
    len = get_file_size()
    f = open(file_name,"rb")
    data = f.read(len)
    f.close()
    file_crc = calculate_crc32(bytearray(data),len)
    print(f"file crc : {file_crc}")
    return file_crc

""" ================ Calculate File size ================ """    

def get_file_size():
    len = os.path.getsize(file_name)
    print("file size: ", len)
    return len

""" ================ Write to Serial port ================ """
def serial_write(data):
    val = struct.pack('>B',data)
    #val = bytearray(val)
    # if mem_write_flag:
    #     print(" "+"0x{:02x}".format(val[0]), end =' ')
    ser.write(val)

""" ================ Read Response Packet ================ """
def receive_packet():
    bytes = ser.readline().decode("utf-8")
    return bytes

""" ================ Convert word to byte ================ """
def word_to_byte(data, pos):
    val= (((data) >> (pos * 8)) & 0xFF)
    return val

""" ================ Start Packet ================ """
def send_start_packet():
    data = list()
    len_to_follow = CRC_LEN  + 1  
    data.append(SOF)
    data.append(OTA_PACKET_Type.OTA_PACKET_CMD)
    data.append(word_to_byte(len_to_follow,0))   #lsb
    data.append(word_to_byte(len_to_follow,1))   #MSB
    data.append(OTA_STATE_Type.OTA_STATE_START)
    crc = calculate_crc32(bytearray(data),len(data))
    data.append(word_to_byte(crc,0))
    data.append(word_to_byte(crc,1))
    data.append(word_to_byte(crc,2))
    data.append(word_to_byte(crc,3))
    print(data)
    for d in data:
        serial_write(d)

""" ================ End Packet ================ """
def send_end_packet():
    data = list()
    len_to_follow = CRC_LEN + 1
    data.append(SOF)
    data.append(OTA_PACKET_Type.OTA_PACKET_CMD)
    data.append(word_to_byte(len_to_follow,0))   #lsb
    data.append(word_to_byte(len_to_follow,1))   #MSB
    data.append(OTA_STATE_Type.OTA_STATE_END)
    crc = calculate_crc32(bytearray(data),len(data))
    data.append(word_to_byte(crc,0))
    data.append(word_to_byte(crc,1))
    data.append(word_to_byte(crc,2))
    data.append(word_to_byte(crc,3))
    print(data)
    for d in data:
        serial_write(d)
    sleep(0.5)
    str = receive_packet()
    if str == "ack":
        print("ack")

""" ================ Abort packet ================ """
def send_abort_packet():
    data = list()
    len_to_follow = CRC_LEN + 1
    data.append(SOF)
    data.append(OTA_PACKET_Type.OTA_PACKET_CMD)
    data.append(word_to_byte(len_to_follow,0))   #lsb
    data.append(word_to_byte(len_to_follow,1))   #MSB
    data.append(OTA_STATE_Type.OTA_STATE_ABORT)
    crc = calculate_crc32(bytearray(data),len(data))
    data.append(word_to_byte(crc,0))
    data.append(word_to_byte(crc,1))
    data.append(word_to_byte(crc,2))
    data.append(word_to_byte(crc,3))
    print(data)
    for d in data:
        serial_write(d)
    sleep(0.5)
    str = receive_packet()
    if str == "ack":
        print("ack")

""" ================ Header Packet ================ """
def send_header_packet():
    data = list()
    len_to_follow = CRC_LEN + 4 + 4
    data.append(SOF)
    data.append(OTA_PACKET_Type.OTA_PACKET_HEADER)
    data.append(word_to_byte(len_to_follow,0))   #lsb
    data.append(word_to_byte(len_to_follow,1))   #MSB
    total_len = get_file_size()
    # a = total_len // 256
    # b =   0 if(total_len % 256 == 0) else 1
    # num_of_pkt = a + b
    print(f"total_len {total_len}")
    data.append(word_to_byte(total_len,0))
    data.append(word_to_byte(total_len,1))
    data.append(word_to_byte(total_len,2))
    data.append(word_to_byte(total_len,3))

    fw_crc = get_fw_crc()
    print(f"FW CRC {hex(fw_crc)}")
    data.append(word_to_byte(fw_crc,0))
    data.append(word_to_byte(fw_crc,1))
    data.append(word_to_byte(fw_crc,2))
    data.append(word_to_byte(fw_crc,3))

    crc = calculate_crc32(bytearray(data),len(data))
    data.append(word_to_byte(crc,0))
    data.append(word_to_byte(crc,1))
    data.append(word_to_byte(crc,2))
    data.append(word_to_byte(crc,3))

    print(data)
    for d in data:
        serial_write(d)   

    sleep(0.5)
    str = receive_packet()
    if str == "ack":
        print("ack")

""" ================ Data Packet ================ """
def data_packet():
    total_len = get_file_size()
    bytes_to_read = 256
    sent_bytes = 0
    remain_bytes = total_len - sent_bytes
    f = open(file_name,"rb")
    pkt_count = 0
    while remain_bytes:
        if remain_bytes > 256:
            bytes_to_read = 256
        else:
            bytes_to_read = remain_bytes
        data = list()
        len_to_follow = CRC_LEN + bytes_to_read
        data.append(SOF)
        data.append(OTA_PACKET_Type.OTA_PACKET_DATA)
        data.append(len_to_follow & 0xFF)   #lsb
        data.append((len_to_follow >> 8)& 0xFF)   #MSB
        read_bytes = f.read(bytes_to_read)
        for i in (read_bytes):
            data.append(i)
        
        sent_bytes += bytes_to_read
        remain_bytes = total_len - sent_bytes

        crc = calculate_crc32(bytearray(data),len(data))
        data.append(word_to_byte(crc,0))
        data.append(word_to_byte(crc,1))
        data.append(word_to_byte(crc,2))
        data.append(word_to_byte(crc,3))
        print(f"---------------------------------")
        print(f"packet count :{pkt_count}")
        print(f"sent bytes   :{sent_bytes}")
        print(f"remain_bytes :{remain_bytes}")
        print(data)
        for d in data:
            serial_write(d) 
        pkt_count += 1
        sleep(1)  
        str = receive_packet()
        if str == "ack":
            print("ack------------------------------------------") 
        
        # remain_bytes = 0

def read_user_input():
    print("Please Enter Y For Run Q for Quit")
    var = input()
    print(var)
    if var == 'Y' or var == 'y':
        state == OTA_STATE_Type.OTA_STATE_START
    elif var == 'Q' or var == 'q':
        SystemExit()


if __name__ == "__main__":
    # while True:
    #     len = ser.in_waiting
    #     if len:
    #         bytes =  ser.read(len).decode("utf-8")
    #         if bytes == "OTA":
    get_fw_crc()    
    open_com()
    ser.flush() 
    while True:

        if state == OTA_STATE_Type.OTA_STATE_START:
            print("OTA_STATE_START")
            send_start_packet()
            sleep(2)
            ser.flush()
            state = OTA_STATE_Type.OTA_STATE_HEADER

        elif state == OTA_STATE_Type.OTA_STATE_HEADER:
                print("OTA_STATE_HEADER")
                send_header_packet()
                sleep(2)
                ser.flush()
                state = OTA_STATE_Type.OTA_STATE_DATA

        elif state == OTA_STATE_Type.OTA_STATE_DATA: 
                print("OTA_STATE_DATA")           
                data_packet()
                sleep(1)
                ser.flush()
                state = OTA_STATE_Type.OTA_STATE_END

        elif state == OTA_STATE_Type.OTA_STATE_END:
                print("OTA_STATE_END")
                send_end_packet()
                ser.flush()
                close_com() 
                state = OTA_STATE_Type.OTA_STATE_IDLE

        elif state == OTA_STATE_Type.OTA_STATE_ABORT:
            print("OTA_STATE_ABORT")
            send_abort_packet()
            sleep(2)
            ser.flush()
            state = OTA_STATE_Type.OTA_STATE_IDLE
        else:
            print("OTA_STATE_IDLE")
            print("Please Enter (Y/y) For Run or (Q/q) for Quit")
            var = input()
            var.strip()
            print(var)
            if var == 'Y' or var == 'y':
                print("Running OTA Process")
                state = OTA_STATE_Type.OTA_STATE_START
            elif var == 'Q' or var == 'q':
                print("system exit")
                sys.exit(1)
            else:
                print("No match")
            sleep(1)




