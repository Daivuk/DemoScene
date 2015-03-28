extern "C" int _fltused = 0x9875; // has taken from "stub.c" in the CRT sources.

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
