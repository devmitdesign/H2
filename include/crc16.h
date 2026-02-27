#pragma once
#include <stdint.h>
#include <stddef.h>
uint16_t crc16_ccitt_false(const uint8_t* data, size_t len, uint16_t init = 0xFFFF);
