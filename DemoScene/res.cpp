#include <d3d11.h>
#include <cinttypes>
#include "mem.h"
#include "res.h"
#include "mat.h"

extern ID3D11Device* device;

#define PI 3.1415926535897932384626433832795f

ID3D11ShaderResourceView* res_texWhite;

sTexture* res_textures;
sMesh* res_meshes;
sModel* res_models;
sCamera* res_cameras;

int res_textureCount;
int res_meshCount;
int res_modelCount;
int res_cameraCount;

sCamera* res_currentCamera;

float* transforms;

auto vertSize = (3 + 3 + 2);

struct sImgContext
{
    uint32_t* pData;
    int w;
    int h;
} img;

struct sMeshQuad
{
    sMeshQuad *pNext;
    float x, y, z, w, h, angleX, angleZ;
};

struct sMeshContext
{
    sMeshQuad *pQuads;
} mesh;

void textureFromData(sTexture& out, const uint8_t* pData, UINT w, UINT h)
{
    ID3D11Texture2D* pTexture;

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
    device->CreateShaderResourceView(pTexture, NULL, &(out.view));

    out.w = (int)w;
    out.h = (int)h;
}

void meshFromData(sMesh& out, sMeshQuad* pQuad, sTexture* pTexture, sTexture* pNormalMap)
{
    out.texture = pTexture;
    out.normalMap = pNormalMap;

    // Count required data first
    int vertCount = 0;
    int indiceCount = 0;
    sMeshQuad* quadPtr = pQuad;
    while (quadPtr)
    {
        quadPtr = quadPtr->pNext;
        vertCount += 4;
        indiceCount += 6;
    }

    // Allocate
    float* vertices = (float*)mem_alloc((vertCount * vertSize * sizeof(float)) + (indiceCount * sizeof(unsigned short)));
    unsigned short* indices = (unsigned short*)(vertices + (vertCount * vertSize));

    // Fill data
    vertCount = 0;
    indiceCount = 0;
    int ind;
    while (pQuad)
    {
        ind = (vertCount + 0) * vertSize;
        vertices[ind + 0] = pQuad->x;
        vertices[ind + 1] = pQuad->y;
        vertices[ind + 2] = pQuad->z + pQuad->h;
        vertices[ind + 3] = 0;
        vertices[ind + 4] = -1;
        vertices[ind + 5] = 0;
        vertices[ind + 6] = 0;
        vertices[ind + 7] = 0;

        ind = (vertCount + 1) * vertSize;
        vertices[ind + 0] = pQuad->x;
        vertices[ind + 1] = pQuad->y;
        vertices[ind + 2] = pQuad->z;
        vertices[ind + 3] = 0;
        vertices[ind + 4] = -1;
        vertices[ind + 5] = 0;
        vertices[ind + 6] = 0;
        vertices[ind + 7] = 1;

        ind = (vertCount + 2) * vertSize;
        vertices[ind + 0] = pQuad->x + pQuad->w;
        vertices[ind + 1] = pQuad->y;
        vertices[ind + 2] = pQuad->z;
        vertices[ind + 3] = 0;
        vertices[ind + 4] = -1;
        vertices[ind + 5] = 0;
        vertices[ind + 6] = 1;
        vertices[ind + 7] = 1;

        ind = (vertCount + 3) * vertSize;
        vertices[ind + 0] = pQuad->x + pQuad->w;
        vertices[ind + 1] = pQuad->y;
        vertices[ind + 2] = pQuad->z + pQuad->h;
        vertices[ind + 3] = 0;
        vertices[ind + 4] = -1;
        vertices[ind + 5] = 0;
        vertices[ind + 6] = 1;
        vertices[ind + 7] = 0;

        indices[indiceCount + 0] = vertCount + 0;
        indices[indiceCount + 1] = vertCount + 1;
        indices[indiceCount + 2] = vertCount + 2;
        indices[indiceCount + 3] = vertCount + 0;
        indices[indiceCount + 4] = vertCount + 2;
        indices[indiceCount + 5] = vertCount + 3;

        pQuad = pQuad->pNext;
        indiceCount += 6;
        vertCount += 4;
    }

    // Set up the description of the static vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.ByteWidth = (vertCount * vertSize * sizeof(float));
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&vertexBufferDesc, &vertexData, &out.vertexBuffer);

    // Set up the description of the static index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = (indiceCount * sizeof(unsigned short));
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&indexBufferDesc, &indexData, &out.indexBuffer);

    out.indexCount = (UINT)indiceCount;
}

float dataToPos(uint8_t* pData)
{
    return (float)(*(uint16_t*)pData) * 8.f;
}

float dataToAngle(uint8_t angle)
{
    return (float)angle / 100.f * PI;
}

void quad(float x, float y, float z, int w, int h, float angleX, float angleZ)
{
    auto pQuad = (sMeshQuad*)mem_alloc(sizeof(sMeshQuad));

    pQuad->x = x;
    pQuad->y = y;
    pQuad->z = z;
    pQuad->w = (float)w;
    pQuad->h = (float)h;
    pQuad->angleX = angleX;
    pQuad->angleZ = angleZ;

    if (mesh.pQuads)
    {
        mesh.pQuads->pNext = pQuad;
    }
    else
    {
        mesh.pQuads = pQuad;
    }
}

void modelFromMeshes(sModel& out, float *transform, float x, float y, float z, float angleX, float angleY, int meshCount, uint8_t* meshes)
{
    out.count = meshCount;
    out.meshes = (sMesh**)mem_alloc(sizeof(sMesh*) * meshCount);
    out.transform = transform;
    mat_identity(out.transform);
    mat_translate(out.transform, x, y, z);
    for (int i = 0; i < meshCount; ++i)
    {
        out.meshes[i] = res_meshes + meshes[i];
    }
}

void setupCamera(sCamera& out, float *pos, float* lookat, float* transform, float x, float y, float z, float tx, float ty, float tz)
{
    out.pos = pos;
    out.lookAt = lookat;
    out.transform = transform;
    out.pos[0] = x;
    out.pos[1] = y;
    out.pos[2] = z;
    out.lookAt[0] = tx;
    out.lookAt[1] = ty;
    out.lookAt[2] = tz;

    res_currentCamera = &out;
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

void createImg(int w, int h)
{
    img.pData = (uint32_t*)mem_alloc(w * h * 4);
    img.w = w;
    img.h = h;
}

void createMesh()
{
    mesh.pQuads = nullptr;
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

void drawCircle(int cx, int cy, int radius, uint32_t color, int edgeSize = EDGE_SIZE)
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

            int nx = p - px;
            int ny = p - py;
            nx = (nx + 255) / 2;
            ny = (ny + 255) / 2;
            int nz = 255 - (255 - nx) * (255 - ny) / 255;

            pNewData[k] = 
                0xff000000 |
                ((nz << 16) & 0xff0000) |
                ((ny << 8) & 0xff00) | 
                (nx & 0xff);
        }
    }
    mem_cpy(img.pData, pNewData, img.w * img.h * 4);
}

enum eRES_CMD : uint8_t
{
    RES_IMG,
    RES_FILL,
    RES_RECT,
    RES_BEVEL,
    RES_CIRCLE,
    RES_BEVEL_CIRCLE,
    RES_LINE,
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
    RES_AMBIENT
};

uint8_t resData[] = {
    3, // Texture count
    1, // Mesh count
    4, // Model count
    1, // Camera count

    RES_IMG, 1, 1,
    RES_FILL, 255, 255, 255, 255,
    RES_IMG_END,

    RES_IMG, 7, 8,
    RES_FILL, 0xc9, 0xc5, 0xa9, 0xff,
    RES_RECT, 0xf9, 0x95, 0x2c, 0xff, 0, 0, 128 / 4, 64 / 4,
    RES_RECT, 0x73, 0x75, 0x68, 0xff, 0, (64 + 128) / 4, 128 / 4, (64 + 128 + 16 + 32) / 4,
    RES_RECT, 0x33, 0x37, 0x3a, 0xff, 0, (64 + 128) / 4, 128 / 4, (64 + 128 + 16) / 4,
    RES_RECT, 0x33, 0x37, 0x3a, 0xff, 0, (64 + 128 + 16 + 32) / 4, 128 / 4, 256 / 4,
    RES_RECT, 0, 0, 0, 0x55, 0, (64 + 128) / 4, 128 / 4, 256 / 4,
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

    RES_IMG, 7, 8,
    RES_FILL, 255, 255, 255, 255,
    RES_RECT, 0, 0, 0, 0x55, 0, (64 + 128) / 4, 128 / 4, 256 / 4,
    RES_BEVEL, 128, 2, 0, 0, 128 / 4, 64 / 4,
    RES_BEVEL, 64, 2, 0, (64 - 20) / 4, 20 / 4, 64 / 4,
    RES_BEVEL, 64, 2, (128 - 20) / 4, (64 - 20) / 4, 128 / 4, 64 / 4,
    RES_BEVEL, 128, 2, 0, 64 / 4, 64 / 4, (64 + 128) / 4,
    RES_BEVEL, 128, 2, 64 / 4, 64 / 4, (64 + 64) / 4, (64 + 128) / 4,
    RES_BEVEL, 255, 8, 0, (64 + 128) / 4, 128 / 4, (64 + 128 + 16) / 4,
    RES_BEVEL, 255, 8, 0, (64 + 128 + 16) / 4, 128 / 4, (64 + 128 + 32) / 4,
    RES_BEVEL, 255, 8, 0, (64 + 128 + 32) / 4, 128 / 4, (64 + 128 + 48) / 4,
    RES_BEVEL, 255, 8, 0, (64 + 128 + 48) / 4, 128 / 4, (64 + 128 + 64) / 4,
    RES_BEVEL_CIRCLE, 0, 0, 0, 0xff, 8 / 4, (64 + 8) / 4, 3, 2,
    RES_BEVEL_CIRCLE, 0, 0, 0, 0xff, 20 / 4, (64 + 8) / 4, 3, 2,
    RES_BEVEL_CIRCLE, 0, 0, 0, 0xff, 8 / 4, (64 + 128 - 8) / 4, 2, 1,
    RES_BEVEL_CIRCLE, 0, 0, 0, 0xff, 20 / 4, (64 + 128 - 8) / 4, 2, 1,
    RES_NORMAL_MAP,
    RES_IMG_END,

    //RES_IMG, 8, 8,
    //RES_CIRCLE, 255, 255, 255, 255, 128 / 4, 128 / 4, 96 + 4,
    //RES_CIRCLE, 255, 0, 0, 255, 128 / 4, 128 / 4, 96,
    //RES_LINE, 255, 255, 255, 255, 64 / 4, 128 / 4, 192 / 4, 128 / 4, 16,
    //RES_IMG_END,

    RES_MESH,
    RES_QUAD, 0, 0, 0, 0, 0, 0, 7, 8, 0, 0, 
    RES_MESH_END, 1, 2,

    // Map layout
    RES_MODEL, 0, 0, 64, 0, 0, 0, 0, 0, 1, 0,
    RES_MODEL, 16, 0, 64, 0, 0, 0, 0, 0, 1, 0,
    RES_MODEL, 32, 0, 64, 0, 0, 0, 0, 0, 1, 0,
    RES_MODEL, 48, 0, 64, 0, 0, 0, 0, 0, 1, 0,

    // Cameras
    RES_CAMERA, 32, 0, 0, 0, 32, 0, 0, 0, 64, 0, 16, 0,
};

void res_load()
{
    res_textureCount = resData[0];
    res_meshCount = resData[1];
    res_modelCount = resData[2];
    res_cameraCount = resData[3];

    int curTexture = 0;
    int curMesh = 0;
    int curModel = 0;
    int curCamera = 0;

    res_textures = (sTexture*)mem_alloc(sizeof(sTexture) * res_textureCount);
    res_meshes = (sMesh*)mem_alloc(sizeof(sMesh) * res_meshCount);
    res_models = (sModel*)mem_alloc(sizeof(sModel) * res_modelCount);
    res_cameras = (sCamera*)mem_alloc(sizeof(sCamera) * res_cameraCount);

    transforms = (float*)mem_alloc(sizeof(float) * (
                                   res_modelCount * 16 + 
                                   res_cameraCount * (3 + 3 + 16)
                                   ));
    float *pTransforms = transforms;

    for (int i = 4; i < sizeof(resData); ++i)
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
            case RES_BEVEL_CIRCLE:
                drawCircle((int)(resData[i + 5]) * 4, 
                           (int)(resData[i + 6]) * 4, 
                           resData[i + 7], 
                           *(uint32_t*)(resData + i + 1),
                           (int)(resData[i + 8]));
                i += 8;
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
                textureFromData(res_textures[curTexture++], (uint8_t*)img.pData, img.w, img.h);
                break;
            case RES_NORMAL_MAP:
                normalMap();
                break;

            case RES_MESH:
                createMesh();
                break;
            case RES_QUAD:
                quad(dataToPos(resData + i + 1),
                     dataToPos(resData + i + 3), 
                     dataToPos(resData + i + 5),
                     1 << resData[i + 7],
                     1 << resData[i + 8],
                     dataToAngle(resData[i + 9]),
                     dataToAngle(resData[i + 10]));
                i += 10;
                break;
            case RES_MESH_END:
                meshFromData(res_meshes[curMesh++], mesh.pQuads, &res_textures[resData[i + 1]], &res_textures[resData[i + 2]]);
                i += 2;
                break;

            case RES_MODEL:
                modelFromMeshes(res_models[curModel],
                                pTransforms,
                                dataToPos(resData + i + 1),
                                dataToPos(resData + i + 3),
                                dataToPos(resData + i + 5),
                                dataToAngle(resData[i + 7]),
                                dataToAngle(resData[i + 8]),
                                (int)resData[i + 9], resData + i + 10);
                i += 9 + (int)resData[i + 9];
                ++curModel;
                pTransforms += 16;
                break;

            case RES_CAMERA:
                setupCamera(res_cameras[curCamera],
                            pTransforms,
                            pTransforms + 3,
                            pTransforms + 6,
                            dataToPos(resData + i + 1),
                            dataToPos(resData + i + 3),
                            dataToPos(resData + i + 5),
                            dataToPos(resData + i + 7),
                            dataToPos(resData + i + 9),
                            dataToPos(resData + i + 11));
                pTransforms += 3 + 3 + 16;
                break;
        }
    }
}
