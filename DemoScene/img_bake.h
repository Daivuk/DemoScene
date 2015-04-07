#pragma once
#include <cinttypes>
#ifdef EDITOR
#include <memory>
#define mem_alloc malloc
#define mem_cpy memcpy
extern double sqrt14(double n);
#endif

enum eRES_CMD : uint8_t
{
    RES_IMG,
    RES_FILL,
    RES_RECT,
    RES_BEVEL,
    RES_CIRCLE,
    RES_BEVEL_CIRCLE,
    RES_LINE,
    RES_GRADIENT,
    RES_NORMAL_MAP,
    RES_IMG_END,

    RES_MESH,
    RES_QUAD,
    RES_MESH_END,

    RES_MODEL,
    RES_CAMERA,
    RES_EMITTER,
    RES_LIGHT,
    RES_SPOT_LIGHT,
    RES_SUN_LIGHT,
    RES_AMBIENT,

    RES_IMAGE,
    RES_END,
};

struct sImgContext
{
    uint32_t* pData;
    int w;
    int h;
};

extern sImgContext img;

int unpackPos(uint8_t pos);
uint32_t blendColors(uint32_t src, uint32_t dst);
void fill(uint32_t color);
int pow(int val, int exp);
void drawCircle(int cx, int cy, int radius, uint32_t color, int edgeSize = 1);
int dot(int x1, int y1, int x2, int y2);
int distance(int x1, int y1, int x2, int y2);
void drawLine(int fromX, int fromY, int toX, int toY, uint32_t color, int thick);
int clamp(int x, int min, int max);
void fillRect(uint32_t color, int fromX, int fromY, int toX, int toY);
void bevel(uint32_t color, int size, int fromX, int fromY, int toX, int toY);
void normalMap();
void putImg(uint32_t color, int fromX, int fromY, int toX, int toY, uint32_t* pSrc, int srcW, int srcH);
