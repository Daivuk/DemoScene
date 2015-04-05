#pragma once
#include <cinttypes>

uint8_t* decompress(uint8_t* srcData, int srcSize, int& outSize);
#if EDITOR
uint8_t* compress(uint8_t* srcData, int srcSize, int& outSize);
#endif
