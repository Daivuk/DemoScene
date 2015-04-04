#include "graphics.h"
#include "ds_mem.h"
#include "mat.h"

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

void mat_v3sub(float* v1, float* v2, float* out)
{
    out[0] = v1[0] - v2[0];
    out[1] = v1[1] - v2[1];
    out[2] = v1[2] - v2[2];
}

void mat_v3normalize(float* v)
{
    float len = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    len = (float)sqrt14((double)len);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}

void mat_v3cross(float* v1, float* v2, float* out)
{
    out[0] = v1[1] * v2[2] - v1[2] * v2[1];
    out[1] = v1[2] * v2[0] - v1[0] * v2[2];
    out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

float mat_v3dot(float* v1, float* v2)
{
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void mat_lookAt(float* m, float* pos, float* dir, float* in_up)
{
    float front[3] = {dir[0], dir[1], dir[2]};
    float right[3];
    float up[3];
    float eye[3] = {-pos[0], -pos[1], -pos[2]};

    mat_v3cross(front, in_up, right);
    mat_v3cross(right, front, up);

    mat_v3normalize(front);
    mat_v3normalize(right);
    mat_v3normalize(up);

    front[0] = -front[0];
    front[1] = -front[1];
    front[2] = -front[2];

    m[0] = right[0];
    m[1] = right[1];
    m[2] = right[2];
    m[3] = mat_v3dot(right, eye);
    m[4] = up[0];
    m[5] = up[1];
    m[6] = up[2];
    m[7] = mat_v3dot(up, eye);
    m[8] = front[0];
    m[9] = front[1];
    m[10] = front[2];
    m[11] = mat_v3dot(front, eye);
    m[12] = 0.f;
    m[13] = 0.f;
    m[14] = 0.f;
    m[15] = 1.f;
}
