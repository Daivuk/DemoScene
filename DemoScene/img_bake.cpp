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

void fill(uint32_t color)
{
    int size = img.w * img.h;
    auto pImg = img.pData;
    while (size)
    {
        *pImg = blendColors(color, *pImg);
        --size;
        ++pImg;
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

#define EDGE_SIZE 1

void drawCircle(int cx, int cy, int radius, uint32_t color, int edgeSize)
{
    int x = 0, y = 0;
    int i = 0;
    int percent;
    while (i < img.w * img.h)
    {
        x = i % img.w;
        y = i / img.w;
        int dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);
        if (dist <= radius * radius)
        {
            uint32_t col = color;

            if (dist >= (radius - edgeSize) * (radius - edgeSize) && radius > edgeSize)
            {
                percent = dist - (radius - edgeSize) * (radius - edgeSize);
                percent *= 255;
                percent = 255 - percent / ((radius * radius) - (radius - edgeSize) * (radius - edgeSize));
            }
            else
            {
                percent = 255;
            }

            percent = percent * ((color >> 24) & 0xff) / 255;
            col = (color & 0x00ffffff) | ((percent << 24) & 0xff000000);

            *(img.pData + i) = blendColors(col, *(img.pData + i));
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
              uint32_t color, int thick)
{
    int x = 0, y = 0;
    int i = 0;
    int segLen = distance(fromX, fromY, toX, toY);
    int percent;
    while (i < img.w * img.h)
    {
        x = i % img.w;
        y = i / img.w;
        int t = dot(x - fromX, y - fromY, toX - fromX, toY - fromY);
        int dist;
        if (t < 0 || segLen == 0)
        {
            dist = distance(x, y, fromX, fromY);
        }
        else if (t > segLen)
        {
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
            uint32_t col = color;

            if (dist >= (thick - EDGE_SIZE) * (thick - EDGE_SIZE) && thick > EDGE_SIZE)
            {
                percent = dist - (thick - EDGE_SIZE) * (thick - EDGE_SIZE);
                percent *= 255;
                percent = 255 - percent / ((thick * thick) - (thick - EDGE_SIZE) * (thick - EDGE_SIZE));
            }
            else
            {
                percent = 255;
            }

            percent = percent * ((color >> 24) & 0xff) / 255;
            col = (color & 0x00ffffff) | ((percent << 24) & 0xff000000);

            *(img.pData + i) = blendColors(col, *(img.pData + i));
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
            img.pData[y * img.w + x] = blendColors(color, img.pData[y * img.w + x]);
        }
    }
}

void putImg(uint32_t color, int fromX, int fromY, int toX, int toY, uint32_t* pSrc, int srcW, int srcH)
{
    fromX = clamp(fromX, 0, img.w);
    fromY = clamp(fromY, 0, img.h);
    toX = clamp(toX, 0, img.w);
    toY = clamp(toY, 0, img.h);
    for (int y = fromY; y < toY; ++y)
    {
        for (int x = fromX; x < toX; ++x)
        {
            int srcX = (x - fromX) % srcW;
            int srcY = (y - fromY) % srcH;
            uint32_t src = pSrc[srcY * srcW + srcX];
            src = 
                ((src & 0xff) * (color & 0xff) / 255) |
                (((((src >> 8) & 0xff) * ((color >> 8) & 0xff) / 255) << 8) & 0xff00) |
                (((((src >> 16) & 0xff) * ((color >> 16) & 0xff) / 255) << 16) & 0xff0000) |
                (((((src >> 24) & 0xff) * ((color >> 24) & 0xff) / 255) << 24) & 0xff000000);
            img.pData[y * img.w + x] = blendColors(src, img.pData[y * img.w + x]);
        }
    }
}

void bevel(uint32_t color, int size, int fromX, int fromY, int toX, int toY)
{
    uint32_t col;
    for (int y = fromY; y < toY; ++y)
    {
        if (y < 0) continue;
        if (y >= img.h) return;
        for (int x = fromX; x < toX; ++x)
        {
            if (x < 0) continue;
            if (x >= img.w) break;
            if (x >= fromX && x < toX && y >= fromY && y < toY)
            {
                int dist = x - fromX;
                if (y - fromY < dist) dist = y - fromY;
                if (toX - x - 1 < dist) dist = toX - x - 1;
                if (toY - y - 1 < dist) dist = toY - y - 1;
                if (dist < size)
                {
                    int k = y * img.w + x;
                    int percent = 255 - dist * 255 / size;
                    //    percent = percent * percent / 255;

                    percent = percent * ((color >> 24) & 0xff) / 255;
                    col = (color & 0x00ffffff) | ((percent << 24) & 0xff000000);

                    img.pData[k] = blendColors(col, img.pData[k]);
                }
            }
        }
    }
}

#define NORMAL_STRENGTH 2;

void normalMap()
{
    auto pNewData = (uint32_t*)mem_alloc(img.w * img.h * 4); // We will waste that. who cares?
    for (int y = 0; y < img.h; ++y)
    {
        for (int x = 0; x < img.w; ++x)
        {
            int k = y * img.w + x;
            int kx = y * img.w + ((x + 1) % img.w);
            int ky = ((y + 1) % img.h) * img.w + x;

            int p = img.pData[k] & 0xff;
            int px = img.pData[kx] & 0xff;
            int py = img.pData[ky] & 0xff;

            int nx = (p - px) * NORMAL_STRENGTH;
            int ny = (p - py) * NORMAL_STRENGTH;
            int nz = 255;

            // Normalize
#ifdef EDITOR
            int len = (int)std::sqrt((double)(nx * nx + ny * ny + nz * nz));
#else
            int len = (int)sqrt14((double)(nx * nx + ny * ny + nz * nz));
#endif
            nx = nx * 255 / len;
            ny = ny * 255 / len;
            nz = nz * 255 / len;

            // Bring back in range
            nx = (nx + 255) / 2;
            ny = (ny + 255) / 2;
            nz = (nz + 255) / 2;

            pNewData[k] =
                0xff000000 |
                ((nz << 16) & 0xff0000) |
                ((ny << 8) & 0xff00) |
                (nx & 0xff);
        }
    }
    mem_cpy(img.pData, pNewData, img.w * img.h * 4);
}
