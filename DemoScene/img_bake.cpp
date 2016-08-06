#pragma once
#include <cinttypes>
#include "img_bake.h"
#ifdef EDITOR
#include <cmath>
#else
#include "ds_mem.h"
#include "mat.h"
#endif

sImgContext img;

int unpackPos(uint8_t pos)
{
    return (int)pos * 4 - 32;
}

uint32_t blendColors(uint32_t src, uint32_t dst)
{
    uint32_t rSrc = src & 0xff;
    uint32_t gSrc = (src >> 8) & 0xff;
    uint32_t bSrc = (src >> 16) & 0xff;
    uint32_t aSrc = (src >> 24) & 0xff;
    uint32_t invASrc = 255 - aSrc;

    uint32_t rDst = dst & 0xff;
    uint32_t gDst = (dst >> 8) & 0xff;
    uint32_t bDst = (dst >> 16) & 0xff;
    uint32_t aDst = (dst >> 24) & 0xff;

    rDst = rSrc * aSrc / 255 + rDst * invASrc / 255;
    gDst = gSrc * aSrc / 255 + gDst * invASrc / 255;
    bDst = bSrc * aSrc / 255 + bDst * invASrc / 255;
    aDst = aSrc + aDst;

    if (aDst > 255) aDst = 255;

    return
        (rDst & 0xff) |
        ((gDst << 8) & 0xff00) |
        ((bDst << 16) & 0xff0000) |
        ((aDst << 24) & 0xff000000);
}

enum eTextureChannel
{
    CHANNEL_DIFFUSE = 0,
    CHANNEL_NORMAL = 1,
    CHANNEL_MATERIAL = 2
};

void applyState(int pixelIndex, int edgeDist, uint32_t srcColor)
{
    auto percent = clamp(img.bakeState.bevel - edgeDist, 0, img.bakeState.bevel);
    if (img.bakeState.bevel)
    {
        percent = percent * 255 / img.bakeState.bevel;
    }
    else
    {
        percent = 0;
    }
    if (!img.bakeState.invBevel)
    {
        percent = 255 - percent;
    }
    percent = clamp(percent, 0, 255);

    // Normal
    int32_t normal = (int32_t)img.pData[CHANNEL_NORMAL][pixelIndex];
    normal += img.bakeState.raise * percent / 255;
    img.pData[CHANNEL_NORMAL][pixelIndex] = (uint32_t)normal;

    // Material
    auto specular = clamp(img.bakeState.specular * 255 / 100, 0, 255);
    auto shininess = clamp(img.bakeState.shininess * 255 / 100, 0, 255);
    auto selfIllum = clamp(img.bakeState.selfIllum * 255 / 100, 0, 255);
    uint32_t col =
        (specular & 0x000000ff) |
        ((shininess << 8) & 0x0000ff00) |
        ((selfIllum << 16) & 0x00ff0000) |
        ((percent << 24) & 0xff000000);
    img.pData[CHANNEL_MATERIAL][pixelIndex] = blendColors(col, img.pData[CHANNEL_MATERIAL][pixelIndex]);

    // Diffuse
    percent = ((srcColor >> 24) & 0xff) * percent / 255;
    col = (srcColor & 0x00ffffff) | ((percent << 24) & 0xff000000);
    img.pData[CHANNEL_DIFFUSE][pixelIndex] = blendColors(col, img.pData[CHANNEL_DIFFUSE][pixelIndex]);
}

void fill(uint32_t color)
{
    int size = img.w * img.h;
    for (int i = 0; i < size; ++i)
    {
        applyState(i, 0, color);
    }
}

int pow(int val, int exp)
{
    int ret = val;
    while (--exp)
    {
        ret *= val;
    }
    return ret;
}

void drawCircle(int cx, int cy, int radius, uint32_t color)
{
    int x = 0, y = 0;
    int i = 0;
    while (i < img.w * img.h)
    {
        x = i % img.w;
        y = i / img.w;
        int dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);
        if (dist <= radius * radius)
        {
#ifdef EDITOR
            dist = (int)std::sqrt((double)dist);
#else
            dist = (int)sqrt14((double)dist);
#endif
            applyState(i, radius - dist, color);
        }
        ++i;
    }
}

int dot(int x1, int y1, int x2, int y2)
{
    return x1 * x2 + y1 * y2;
}

int distance(int x1, int y1, int x2, int y2)
{
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

void drawLine(int fromX, int fromY,
              int toX, int toY,
              uint32_t color, int thick, bool roundCorners)
{
    int x = 0, y = 0;
    int i = 0;
    int segLen = distance(fromX, fromY, toX, toY);
    while (i < img.w * img.h)
    {
        x = i % img.w;
        y = i / img.w;
        int t = dot(x - fromX, y - fromY, toX - fromX, toY - fromY);
        int dist;
        if (t < 0 || segLen == 0)
        {
            if (!roundCorners) continue;
            dist = distance(x, y, fromX, fromY);
        }
        else if (t > segLen)
        {
            if (!roundCorners) continue;
            dist = distance(x, y, toX, toY);
        }
        else
        {
            int projX = fromX + t * (toX - fromX) / segLen;
            int projY = fromY + t * (toY - fromY) / segLen;
            dist = distance(projX, projY, x, y);
        }
        if (dist <= thick * thick)
        {
#ifdef EDITOR
            dist = (int)std::sqrt((double)dist);
#else
            dist = (int)sqrt14((double)dist);
#endif
            applyState(i, thick - dist, color);
        }
        ++i;
    }
}

int clamp(int x, int min, int max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void fillRect(uint32_t color, int fromX, int fromY, int toX, int toY)
{
    fromX = clamp(fromX, 0, img.w);
    fromY = clamp(fromY, 0, img.h);
    toX = clamp(toX, 0, img.w);
    toY = clamp(toY, 0, img.h);
    for (int y = fromY; y < toY; ++y)
    {
        for (int x = fromX; x < toX; ++x)
        {
            int dist = x - fromX;
            if (y - fromY < dist) dist = y - fromY;
            if (toX - x - 1 < dist) dist = toX - x - 1;
            if (toY - y - 1 < dist) dist = toY - y - 1;
            applyState(y * img.w + x, dist, color);
        }
    }
}

void putImg(uint32_t color, int fromX, int fromY, int toX, int toY, 
            uint32_t* pSrcDiffuse, 
            uint32_t* pSrcNormal,
            uint32_t* pSrcMaterial,
            int srcW, int srcH)
{
    fromX = clamp(fromX, 0, img.w);
    fromY = clamp(fromY, 0, img.h);
    toX = clamp(toX, 0, img.w);
    toY = clamp(toY, 0, img.h);
    auto prevState = img.bakeState;
    for (int y = fromY; y < toY; ++y)
    {
        for (int x = fromX; x < toX; ++x)
        {
            int srcX = (x - fromX) % srcW;
            int srcY = (y - fromY) % srcH;
            uint32_t src = pSrcDiffuse[srcY * srcW + srcX];
            int32_t normal = pSrcNormal[srcY * srcW + srcX];
            uint32_t material = pSrcMaterial[srcY * srcW + srcX] * 100;
            src =
                ((src & 0xff) * (color & 0xff) / 255) |
                (((((src >> 8) & 0xff) * ((color >> 8) & 0xff) / 255) << 8) & 0xff00) |
                (((((src >> 16) & 0xff) * ((color >> 16) & 0xff) / 255) << 16) & 0xff0000) |
                (((((src >> 24) & 0xff) * ((color >> 24) & 0xff) / 255) << 24) & 0xff000000);
            img.bakeState = prevState;
            img.bakeState.raise += normal;
            img.bakeState.specular = (material & 0xff) * 100 / 255;
            img.bakeState.shininess = ((material >> 8) & 0xff) * 100 / 255;
            img.bakeState.selfIllum = ((material >> 16) & 0xff) * 100 / 255;
            applyState(y * img.w + x, 0, src);
            img.bakeState = prevState;
        }
    }
}

#define NORMAL_STRENGTH 1

void normalMap()
{
    int s[9];
    int *pS;
    int avg;
#ifdef EDITOR
    auto pNewData = new uint32_t[img.w * img.h];
#else
    auto pNewData = (uint32_t*)mem_alloc(img.w * img.h * 4);
#endif
    for (int y = 0; y < img.h; ++y)
    {
        for (int x = 0; x < img.w; ++x)
        {
            pS = s;
            for (int j = y + img.h - 1; j <= y + img.h + 1; ++j)
            {
                for (int i = x + img.w - 1; i <= x + img.w + 1; ++i)
                {
                    *pS = img.pData[CHANNEL_NORMAL][(j % img.h) * img.w + (i % img.w)];
                    *pS &= 0xff;
                    ++pS;
                }
            }

            int nx = NORMAL_STRENGTH * -(s[2] - s[0] + 2 * (s[5] - s[3]) + s[8] - s[6]);
            int ny = NORMAL_STRENGTH * -(s[6] - s[0] + 2 * (s[7] - s[1]) + s[8] - s[2]);
            int nz = 255;

            // Normalize
#ifdef EDITOR
            int len = (int)std::sqrt((double)(nx * nx + ny * ny + nz * nz));
#else
            int len = (int)sqrt14((double)(nx * nx + ny * ny + nz * nz));
#endif
            if (len > 0)
            {
                nx = nx * 255 / len;
                ny = ny * 255 / len;
                nz = nz * 255 / len;

                // Bring back in range
                nx = (nx + 255) / 2;
                ny = (ny + 255) / 2;
                nz = (nz + 255) / 2;
            }
            else
            {
                nx = 128;
                ny = 128;
                nz = 255;
            }

            // Ambient, a bit slower. Sample 5x5
            avg = 0;
            for (int j = y + img.h - 4; j <= y + img.h + 4; ++j)
            {
                for (int i = x + img.w - 4; i <= x + img.w + 4; ++i)
                {
                    avg += img.pData[CHANNEL_NORMAL][(j % img.h) * img.w + (i % img.w)] & 0xff;
                }
            }
            avg /= 9 * 9;
            avg = avg - s[4];
            avg = avg * 350 / 255;
            avg = 255 - clamp(avg, 0, 255);
            pNewData[y * img.w + x] =
                ((avg << 24) & 0xff000000) |
                ((nz << 16) & 0xff0000) |
                ((ny << 8) & 0xff00) |
                (nx & 0xff);
        }
    }
    mem_cpy(img.pData[CHANNEL_NORMAL], pNewData, img.w * img.h * 4);
#ifdef EDITOR
    delete[] pNewData;
#endif
}
