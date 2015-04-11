#include <d3d11.h>
#include <cinttypes>
#include "ds_mem.h"
#include "res.h"
#include "mat.h"
#include "img_bake.h"
#include "compress.h"

extern ID3D11Device* device;

#define PI 3.1415926535897932384626433832795f

ID3D11ShaderResourceView* res_texWhite;

sTexture* res_textures;
sMesh* res_meshes;
sModel* res_models;
sCamera* res_cameras;
uint8_t* res_palette;

int res_textureCount;
int res_meshCount;
int res_modelCount;
int res_cameraCount;
int res_colorCount;

sCamera* res_currentCamera;

float* transforms;

auto vertSize = (3 + 3 + 2);

struct sMeshQuad
{
    sMeshQuad *pNext;
    float x, y, z, w, h, angleX, angleZ;
};

struct sMeshContext
{
    sMeshQuad *pQuads;
} mesh;

void textureFromData(sTexture& out, const uint8_t* pData, UINT w, UINT h, int channel)
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
    device->CreateShaderResourceView(pTexture, NULL, &(out.view[channel]));

    out.w = (int)w;
    out.h = (int)h;
    out.data[channel] = (uint32_t*)pData;
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

void createMesh()
{
    mesh.pQuads = nullptr;
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

void createImg()
{
    int w = readBits(3) + 3;
    int h = readBits(3) + 3;
    w = pow(2, w);
    h = pow(2, h);
    img.pData[0] = (uint32_t*)mem_alloc(w * h * 4);
    img.pData[1] = (uint32_t*)mem_alloc(w * h * 4);
    img.pData[2] = (uint32_t*)mem_alloc(w * h * 4);
    img.w = w;
    img.h = h;
    img.bakeState.bevel = 0;
    img.bakeState.invBevel = false;
    img.bakeState.raise = 0;
    img.bakeState.selfIllum = 0;
    img.bakeState.shininess = 0;
    img.bakeState.specular = 0;
}

uint8_t resDataCompressed[] = {
#include "res_data.h"
 /*   3, // Texture count
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
    RES_CAMERA, 32, 0, 0, 0, 32, 0, 0, 0, 64, 0, 16, 0,*/
};

uint32_t colorFromPalette(int id)
{
    return *(uint32_t*)(res_palette + (id * 4));
}

int g_x1, g_y1, g_x2, g_y2;
void readRect()
{
    g_x1 = unpackPos(readBits(8));
    g_y1 = unpackPos(readBits(8));
    g_x2 = unpackPos(readBits(8));
    g_y2 = unpackPos(readBits(8));
}
void readPosition()
{
    g_x1 = unpackPos(readBits(8));
    g_y1 = unpackPos(readBits(8));
}

void res_load()
{
    // Uncompress it first
    int dataSize;
    readData = decompress(resDataCompressed, sizeof(resDataCompressed), dataSize);
    readPos = 0;

    res_textureCount = readBits(8);
    res_meshCount = readBits(8);
    res_modelCount = readBits(8);
    res_cameraCount = readBits(8);
    res_colorCount = readBits(8);

    int curTexture = 0;
    int curMesh = 0;
    int curModel = 0;
    int curCamera = 0;

    // Allocate
    res_textures = (sTexture*)mem_alloc(sizeof(sTexture) * res_textureCount * 3);
    res_meshes = (sMesh*)mem_alloc(sizeof(sMesh) * res_meshCount);
    res_models = (sModel*)mem_alloc(sizeof(sModel) * res_modelCount);
    res_cameras = (sCamera*)mem_alloc(sizeof(sCamera) * res_cameraCount);
    res_palette = (uint8_t*)mem_alloc(res_colorCount * 4);
    transforms = (float*)mem_alloc(sizeof(float) * (
                                   res_modelCount * 16 + 
                                   res_cameraCount * (3 + 3 + 16)
                                   ));
    float *pTransforms = transforms;

    for (int i = 0; i < res_colorCount * 4; i += 4)
    {
        res_palette[i + 0] = (uint8_t)readBits(8);
        res_palette[i + 1] = (uint8_t)readBits(8);
        res_palette[i + 2] = (uint8_t)readBits(8);
        res_palette[i + 3] = (uint8_t)readBits(8);
    }

    bool bDone = false;
    while (!bDone)
    {
        switch (readBits(8))
        {
            case RES_IMG:
            {
                createImg();
                break;
            }
            case RES_FILL:
            {
                fill(colorFromPalette(readBits(8)));
                break;
            }
            case RES_LINE:
            {
                auto colorId = readBits(8);
                readRect();
                auto thickness = readBits(6) + 1;
                drawLine(g_x1, g_y1, g_x2, g_y2, colorFromPalette(colorId), thickness, true);
                break;
            }
            case RES_RECT:
            {
                auto colorId = readBits(8);
                readRect();
                fillRect(colorFromPalette(colorId), g_x1, g_y1, g_x2, g_y2);
                break;
            }
            case RES_CIRCLE:
            {
                auto colorId = readBits(8);
                readPosition();
                auto radius = readBits(8) + 1;
                drawCircle(g_x1, g_y1, radius, colorFromPalette(colorId));
                break;
            }
            case RES_IMG_END:
            {
                normalMap();
                textureFromData(res_textures[curTexture], (uint8_t*)img.pData[0], img.w, img.h, 0);
                textureFromData(res_textures[curTexture], (uint8_t*)img.pData[1], img.w, img.h, 1);
                textureFromData(res_textures[curTexture], (uint8_t*)img.pData[2], img.w, img.h, 2);
                ++curTexture;
                break;
            }
            case RES_IMAGE:
            {
                auto colorId = readBits(8);
                readRect();
                auto imgId = readBits(8);
                putImg(colorFromPalette(colorId), g_x1, g_y1, g_x2, g_y2,
                       res_textures[imgId].data[0],
                       res_textures[imgId].data[1],
                       res_textures[imgId].data[2],
                       res_textures[imgId].w,
                       res_textures[imgId].h);
                break;
            }
            case RES_MESH:
                //createMesh();
                break;
            case RES_QUAD:
                //quad(dataToPos(resData + i + 1),
                //     dataToPos(resData + i + 3), 
                //     dataToPos(resData + i + 5),
                //     1 << resData[i + 7],
                //     1 << resData[i + 8],
                //     dataToAngle(resData[i + 9]),
                //     dataToAngle(resData[i + 10]));
                break;
            case RES_MESH_END:
                //meshFromData(res_meshes[curMesh++], mesh.pQuads, &res_textures[resData[i + 1]], &res_textures[resData[i + 2]]);
                break;

            case RES_MODEL:
                //modelFromMeshes(res_models[curModel],
                //                pTransforms,
                //                dataToPos(resData + i + 1),
                //                dataToPos(resData + i + 3),
                //                dataToPos(resData + i + 5),
                //                dataToAngle(resData[i + 7]),
                //                dataToAngle(resData[i + 8]),
                //                (int)resData[i + 9], resData + i + 10);
                //++curModel;
                //pTransforms += 16;
                break;

            case RES_CAMERA:
                //setupCamera(res_cameras[curCamera],
                //            pTransforms,
                //            pTransforms + 3,
                //            pTransforms + 6,
                //            dataToPos(resData + i + 1),
                //            dataToPos(resData + i + 3),
                //            dataToPos(resData + i + 5),
                //            dataToPos(resData + i + 7),
                //            dataToPos(resData + i + 9),
                //            dataToPos(resData + i + 11));
                //pTransforms += 3 + 3 + 16;
                break;

            case RES_END:
                bDone = true;
                break;
        }
    }
}
