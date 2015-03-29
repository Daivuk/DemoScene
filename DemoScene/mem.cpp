#include <Windows.h>

HANDLE heapHandle;

void mem_init()
{
    heapHandle = HeapCreate(0, 1024 * 1024 * 64, 0);
}

void mem_zero(void* pData, int size)
{
    unsigned char* ptr = (unsigned char*)pData;
    while (size)
    {
        *ptr = 0;
        ++ptr;
        --size;
    }
}

void* mem_alloc(int size)
{
    return HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, size);
}

void mem_cpy(void* pDst, const void* pSrc, int size)
{
    for (auto i = 0; i < size; ++i)
    {
        ((unsigned char*)pDst)[i] = ((const unsigned char*)pSrc)[i];
    }
}
