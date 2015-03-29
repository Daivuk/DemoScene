#include <d3d11.h>
#include <cinttypes>
#include "mem.h"

extern ID3D11Device* device;

ID3D11ShaderResourceView* res_texWhite;
ID3D11ShaderResourceView** res_textures;
int res_textureCount;

struct sImgContext
{
    uint32_t* pData;
    int w;
    int h;
} img;

ID3D11ShaderResourceView* textureFromData(const uint8_t* pData, UINT w, UINT h)
{
    ID3D11Texture2D* pTexture;
    ID3D11ShaderResourceView* pTextureView;

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = pData;
    data.SysMemPitch = w * 4;
    data.SysMemSlicePitch = 0;

    device->CreateTexture2D(&desc, &data, &pTexture);
    device->CreateShaderResourceView(pTexture, NULL, &pTextureView);

    return pTextureView;
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

void createImg(int w, int h)
{
    img.pData = (uint32_t*)mem_alloc(w * h * 4);
    img.w = w;
    img.h = h;
}

int pow(int val, int exp)
{
    while (exp--)
    {
        val = val * val / 255;
    }
    return val;
}

#define EDGE_SIZE 1

void drawCircle(int cx, int cy, int radius, uint32_t color)
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

            if (dist >= (radius - EDGE_SIZE) * (radius - EDGE_SIZE) && radius > EDGE_SIZE)
            {
                percent = dist - (radius - EDGE_SIZE) * (radius - EDGE_SIZE);
                percent *= 255;
                percent = 255 - percent / ((radius * radius) - (radius - EDGE_SIZE) * (radius - EDGE_SIZE));
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
        if (t < 0)
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

            //int percent = dist * 255 / (thick * thick);
            //percent = pow(percent, thick / 32 + thick / (thick / 5 + 1));
            //percent = 255 - percent;

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

void fillRect(uint32_t color, int fromX, int fromY, int toX, int toY)
{
    for (int y = fromY; y < toY; ++y)
    {
        for (int x = fromX; x < toX; ++x)
        {
            img.pData[y * img.w + x] = blendColors(color, img.pData[y * img.w + x]);
        }
    }
}

void bevel(uint32_t strength, int size, int fromX, int fromY, int toX, int toY)
{
    for (int y = fromY; y < toY; ++y)
    {
        for (int x = fromX; x < toX; ++x)
        {
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
                    percent = percent * percent / 255;
                    percent = percent * strength / 255;
                    uint32_t src = (percent << 24) & 0xff000000;
                    img.pData[k] = blendColors(src, img.pData[k]);
                }
            }
        }
    }
}

#define RES_IMG 0
#define RES_FILL 1
#define RES_RECT 2
#define RES_BEVEL 3
#define RES_CIRCLE 4
#define RES_IMG_END 5
#define RES_LINE 6

uint8_t resData[] = {
    2, // Texture count

    RES_IMG, 7, 8,
    RES_FILL, 0xc9, 0xc5, 0xa9, 0xff,
    RES_RECT, 0xf9, 0x95, 0x2c, 0xff, 0, 0, 128 / 4, 64 / 4,
    RES_RECT, 0x73, 0x75, 0x68, 0xff, 0, (64 + 128) / 4, 128 / 4, (64 + 128 + 16 + 32) / 4,
    RES_RECT, 0x33, 0x37, 0x3a, 0xff, 0, (64 + 128) / 4, 128 / 4, (64 + 128 + 16) / 4,
    RES_RECT, 0x33, 0x37, 0x3a, 0xff, 0, (64 + 128 + 16 + 32) / 4, 128 / 4, 256 / 4,
    RES_BEVEL, 64, 2, 0, 0, 128 / 4, 64 / 4,
    RES_BEVEL, 64, 2, 0, (64 - 20) / 4, 20 / 4, 64 / 4,
    RES_BEVEL, 64, 2, (128 - 20) / 4, (64 - 20) / 4, 128 / 4, 64 / 4,
    RES_BEVEL, 128, 2, 0, 64 / 4, 64 / 4, (64 + 128) / 4,
    RES_BEVEL, 128, 2, 64 / 4, 64 / 4, (64 + 64) / 4, (64 + 128) / 4,
    RES_BEVEL, 128, 3, 0, (64 + 128) / 4, 128 / 4, (64 + 128 + 16) / 4,
    RES_BEVEL, 128, 3, 0, (64 + 128 + 16) / 4, 128 / 4, (64 + 128 + 32) / 4,
    RES_BEVEL, 128, 3, 0, (64 + 128 + 32) / 4, 128 / 4, (64 + 128 + 48) / 4,
    RES_BEVEL, 128, 3, 0, (64 + 128 + 48) / 4, 128 / 4, (64 + 128 + 64) / 4,
    RES_CIRCLE, 0x39, 0x35, 0x32, 0xff, 8 / 4, (64 + 8) / 4, 3,
    RES_CIRCLE, 0x39, 0x35, 0x32, 0xff, 20 / 4, (64 + 8) / 4, 3,
    RES_CIRCLE, 0x39, 0x35, 0x32, 0xff, 8 / 4, (64 + 128 - 8) / 4, 2,
    RES_CIRCLE, 0x39, 0x35, 0x32, 0xff, 20 / 4, (64 + 128 - 8) / 4, 2,
    RES_IMG_END,

    RES_IMG, 8, 8,
    RES_CIRCLE, 255, 255, 255, 255, 128 / 4, 128 / 4, 96 + 4,
    RES_CIRCLE, 255, 0, 0, 255, 128 / 4, 128 / 4, 96,
    RES_LINE, 255, 255, 255, 255, 64 / 4, 128 / 4, 192 / 4, 128 / 4, 16,
    RES_IMG_END,
};

void res_load()
{
    const uint32_t white = 0xffffffff;
    res_texWhite = textureFromData((uint8_t*)&white, 1, 1);

    res_textureCount = *resData;
    res_textures = (ID3D11ShaderResourceView**)mem_alloc(sizeof(ID3D11ShaderResourceView*) * res_textureCount);
    int curTexture = 0;

    for (int i = 1; i < sizeof(resData); ++i)
    {
        switch (resData[i])
        {
            case RES_IMG:
                createImg(1 << resData[i + 1], 1 << resData[i + 2]);
                i += 2;
                break;
            case RES_FILL:
                fill(*(uint32_t*)(resData + i + 1));
                i += 4;
                break;
            case RES_RECT:
                fillRect(*(uint32_t*)(resData + i + 1),
                         (int)(resData[i + 5]) * 4,
                         (int)(resData[i + 6]) * 4,
                         (int)(resData[i + 7]) * 4,
                         (int)(resData[i + 8]) * 4);
                i += 8;
                break;
            case RES_BEVEL:
                bevel((int)(resData[i + 1]),
                      (int)(resData[i + 2]),
                      (int)(resData[i + 3]) * 4,
                      (int)(resData[i + 4]) * 4,
                      (int)(resData[i + 5]) * 4,
                      (int)(resData[i + 6]) * 4);
                i += 6;
                break;
            case RES_CIRCLE:
                drawCircle((int)(resData[i + 5]) * 4, 
                           (int)(resData[i + 6]) * 4, 
                           resData[i + 7], 
                           *(uint32_t*)(resData + i + 1));
                i += 7;
                break;
            case RES_LINE:
                drawLine((int)(resData[i + 5]) * 4,
                         (int)(resData[i + 6]) * 4, 
                         (int)(resData[i + 7]) * 4,
                         (int)(resData[i + 8]) * 4,
                         *(uint32_t*)(resData + i + 1),
                         resData[i + 9]);
                i += 9;
                break;
            case RES_IMG_END:
                res_textures[curTexture++] = textureFromData((uint8_t*)img.pData, img.w, img.h);
                break;
        }
    }
}
