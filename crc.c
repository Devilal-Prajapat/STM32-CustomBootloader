#include <stdio.h>
#include <stdint.h>

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
    printf("crc32: 0x%08x",crc);
    return (crc);
}

int main()
{
    printf("Hello World");
    uint8_t buf = 0;
    calculate_crc32((uint8_t *)&buf, 1);
    return 0;
}
