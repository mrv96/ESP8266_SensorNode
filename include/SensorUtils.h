#pragma once

#include <Arduino.h>

int set_fields(uint8_t firstField, uint8_t nFields, float *arr);
uint32_t calculateCRC32(const uint8_t *data, size_t length);
uint32_t fwVersionToUINT32(char* str);