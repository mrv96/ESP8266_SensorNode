#include <Arduino.h>
#include <ThingSpeak.h>

int set_fields(uint8_t firstField, uint8_t nFields, float *arr) {
  int x = 0;
  
  for(int i=0; i<nFields; i++) x |= ThingSpeak.setField(firstField+i, arr[i]);

  return x;
}

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

uint32_t fwVersionToUINT32(char* str) {
  return str[0]<<24 | str[1]<<16 | str[2]<<8 | str[3];
}