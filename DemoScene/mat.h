#pragma once

double inline __declspec (naked) __fastcall sqrt14(double n)
{
    _asm fld qword ptr[esp + 4]
        _asm fsqrt
    _asm ret 8
}

// Matrix
void mat_identity(float* m);
void mat_translate(float* m, float x, float y, float z);
void mat_rotateX(float* m, float angle);
void mat_rotateZ(float* m, float angle);
void mat_lookAt(float* m, float* pos, float* dir, float* up);

// Vectors
void mat_v3sub(float* v1, float* v2, float* out);
void mat_v3normalize(float* v);
void mat_v3cross(float* v1, float* v2, float* out);
float mat_v3dot(float* v1, float* v2);
