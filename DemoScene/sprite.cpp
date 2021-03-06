#include <d3d11.h>
#include "ds_mem.h"

extern ID3D11Device* device;
extern ID3D11DeviceContext* deviceContext;

ID3D11ShaderResourceView* currentTexture = nullptr;
ID3D11Buffer* vertexBuffer;
ID3D11Buffer* indexBuffer;
D3D11_MAPPED_SUBRESOURCE mappedVertexBuffer;
int spriteCount = 0;
static const unsigned int stride = (2 + 2 + 4) * 4;
static const unsigned int offset = 0;
float* pVerts;

#define MAX_SPRITE_COUNT 300

void mapVSBuffer()
{
    deviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertexBuffer);
    pVerts = (float*)mappedVertexBuffer.pData;
}

void spr_init()
{
    float* vertices = (float*)mem_alloc((MAX_SPRITE_COUNT * 4 * (2 + 2 + 4) * sizeof(float)) + (MAX_SPRITE_COUNT * 6 * sizeof(unsigned short)));
    unsigned short* indices = (unsigned short*)(vertices + MAX_SPRITE_COUNT * 4 * (2 + 2 + 4));
    int offsets[6] = {0, 1, 2, 2, 3, 0};
    for (auto i = 0; i < MAX_SPRITE_COUNT; ++i)
    {
        for (auto j = 0; j < 6; ++j)
        {
            indices[i * 6 + j] = i * 4 + offsets[j];
        }
    }

    // Set up the description of the static vertex buffer.
    D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = MAX_SPRITE_COUNT * 4 * (2 + 2 + 4) * sizeof(float);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the vertex data.
    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

    // Set up the description of the static index buffer.
    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.ByteWidth = MAX_SPRITE_COUNT * sizeof(unsigned short) * 6;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    // Give the subresource structure a pointer to the index data.
    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

    mapVSBuffer();
}

void spr_flush()
{
    if (!spriteCount)
    {
        return; // Nothing to flush
    }

    deviceContext->Unmap(vertexBuffer, 0);

    deviceContext->PSSetShaderResources(0, 1, &currentTexture);
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    deviceContext->DrawIndexed(6 * spriteCount, 0, 0);

    mapVSBuffer();

    spriteCount = 0;
    currentTexture = nullptr;
}

void spr_draw(ID3D11ShaderResourceView* pTexture, const float* rect, const float* color)
{
    if (pTexture != currentTexture)
    {
        spr_flush();
        currentTexture = pTexture;
    }

    for (auto i = 4; i < 36; i += 8)
    {
        mem_cpy(pVerts + i, color, 32);
    }

    *pVerts = *rect;
    *(pVerts + 1) = *(rect + 1);

    *(pVerts + 8) = *rect;
    *(pVerts + 9) = *(rect + 1) + *(rect + 3);

    *(pVerts + 16) = *rect + *(rect + 2);
    *(pVerts + 17) = *(rect + 1) + *(rect + 3);

    *(pVerts + 24) = *rect + *(rect + 2);
    *(pVerts + 25) = *(rect + 1);

    for (auto j = 0; j < 8; ++j)
    {
        pVerts[2 + (j % 2) + ((j / 2) * 8)] = (float)(((j + 1) / 4) % 2);
    }

    ++spriteCount;
    pVerts += 32;

    if (spriteCount == MAX_SPRITE_COUNT)
    {
        spr_flush();
    }
}
