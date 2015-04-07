#pragma once
#include <cinttypes>

// Read
extern uint8_t* readData;
extern int readPos;
extern int readSize;
extern int readBits(int count);
uint8_t* decompress(uint8_t* srcData, int srcSize, int& outSize);

#if EDITOR
// Write (Editor only)
#include <vector>
extern int writePos;
extern std::vector<uint8_t> compressedData;
extern void write(int val, int bitCount);
uint8_t* compress(uint8_t* srcData, int srcSize, int& outSize);
#endif
