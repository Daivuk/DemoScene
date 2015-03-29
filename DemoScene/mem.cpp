#include <Windows.h>

#pragma function(memset)
void * __cdecl memset(void *pTarget, int value, size_t cbTarget)
{
    unsigned char *p = static_cast<unsigned char *>(pTarget);
    while (cbTarget-- > 0)
    {
        *p++ = static_cast<unsigned char>(value);
    }
    return pTarget;
}

HANDLE heapHandle;

void mem_init()
{
    heapHandle = HeapCreate(0, 1024 * 1024 * 64, 0);
}

void mem_zero(void* pData, int size)
{
    memset(pData, 0, size);
    //unsigned char* ptr = (unsigned char*)pData;
    //while (size)
    //{
    //    *ptr = 0;
    //    ++ptr;
    //    --size;
    //}
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
