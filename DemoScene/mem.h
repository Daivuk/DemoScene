#pragma once

extern "C" void * __cdecl memset(void *, int, size_t);
#pragma intrinsic(memset)

void mem_init();
void mem_zero(void* pData, int size);
void* mem_alloc(int size);
void mem_cpy(void* pDst, const void* pSrc, int size);
