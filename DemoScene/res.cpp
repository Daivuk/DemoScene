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

void fill(uint32_t color)
{
    int size = img.w * img.h;
    auto pImg = img.pData;
    while (size)
    {
        *pImg = color;
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
            *(img.pData + i) = color;
        }
        ++i;
    }
}
/*
float minimum_distance(vec2 v, vec2 w, vec2 p)
{
    // Return minimum distance between line segment vw and point p
    const float l2 = length_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
    if (l2 == 0.0) return distance(p, v);   // v == w case
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of point p onto the line. 
    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
    const float t = dot(p - v, w - v) / l2;
    if (t < 0.0) return distance(p, v);       // Beyond the 'v' end of the segment
    else if (t > 1.0) return distance(p, w);  // Beyond the 'w' end of the segment
    const vec2 projection = v + t * (w - v);  // Projection falls on the segment
    return distance(p, projection);
}
*/
void drawLine(int fromX, int fromY,
              int toX, int toY,
              uint32_t color, int thick)
{
    int x = 0, y = 0;
    int i = 0;
    int segLen = (toX - fromX) * (toX - fromX) + (toY - fromY) * (toY - fromY);
    while (i < img.w * img.h)
    {
        x = i % img.w;
        y = i / img.w;
        int dist = (x - fromX) * (x - fromX) + (y - fromY) * (y - fromY);
        if (dist <= thick * thick)
        {
            *(img.pData + i) = color;
        }
        ++i;
    }
}

void res_load()
{
    const uint32_t white = 0xffffffff;
    res_texWhite = textureFromData((uint8_t*)&white, 1, 1);

    createImg(256, 256);
    fill(-1);
    drawCircle(128, 128, 96, 0xff0000ff);
    drawLine(64, 128, 192, 128, -1, 16);
    res_texTest = textureFromData((uint8_t*)img.pData, img.w, img.h);
}
