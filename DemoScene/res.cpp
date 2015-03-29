#include <d3d11.h>
#include <cinttypes>
#include "mem.h"

extern ID3D11Device* device;

ID3D11ShaderResourceView* res_texWhite;
ID3D11ShaderResourceView* res_texTest;

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

void res_load()
{
    const uint32_t white = 0xffffffff;
    res_texWhite = textureFromData((uint8_t*)&white, 1, 1);
    
    //createImg(128, 256);
    //fill(-1);
    //drawCircle(128, 128, 96 + 4, 0xff000000);
    //drawCircle(128, 128, 96, 0xff0000ff);
    //drawLine(64, 128, 192, 128, -1, 16);
    //res_texTest = textureFromData((uint8_t*)img.pData, img.w, img.h);

    createImg(128, 256);
    fill(0xffa9c5c9);
    fillRect(0xff2c98f9, 0, 0, 128, 64);
    fillRect(0xff687573, 0, 64 + 128 + 16, 128, 64 + 128 + 16 + 32);
    fillRect(0xff3a3733, 0, 64 + 128, 128, 64 + 128 + 16);
    fillRect(0xff3a3733, 0, 64 + 128 + 16 + 32, 128, 256);
    fillRect(0xff303534, 0, 64 + 128 + 16 + 32, 128, 256);
    bevel(64, 2, 0, 0, 128, 64);
    bevel(64, 2, 0, 64 - 20, 21, 64);
    bevel(64, 2, 128 - 21, 64 - 20, 128, 64);
    bevel(128, 2, 0, 64, 64, 64 + 128);
    bevel(128, 2, 64, 64, 64 + 64, 64 + 128);
    bevel(128, 3, 0, 64 + 128, 128, 64 + 128 + 16);
    bevel(128, 3, 0, 64 + 128 + 16, 128, 64 + 128 + 32);
    bevel(128, 3, 0, 64 + 128 + 32, 128, 64 + 128 + 48);
    bevel(128, 3, 0, 64 + 128 + 48, 128, 64 + 128 + 64);
    drawCircle(8, 64 + 8, 3, 0xff323539);
    drawCircle(20, 64 + 8, 3, 0xff323539);
    drawCircle(8, 64 + 128 - 8, 2, 0xff323539);
    drawCircle(20, 64 + 128 - 8, 2, 0xff323539);
    res_texTest = textureFromData((uint8_t*)img.pData, img.w, img.h);
}
