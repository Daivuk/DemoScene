#include "graphics.h"
#include "mem.h"

void mat_identity(float* m)
{
    mem_cpy(m, GFX_IDENTITY, 64);
}

void mat_translate(float* m, float x, float y, float z)
{
    m[3] += x;
    m[7] += y;
    m[11] += z;
}

void mat_rotateX(float* m, float angle)
{
}

void mat_rotateZ(float* m, float angle)
{
}
