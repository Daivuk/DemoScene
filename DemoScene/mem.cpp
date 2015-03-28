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
    return HeapAlloc(heapHandle, 0, size);
}
