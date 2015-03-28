#include <d3d11.h>

extern ID3D11Device* device;

ID3D11ShaderResourceView* res_texWhite;

ID3D11ShaderResourceView* textureFromData(const unsigned char* pData, UINT w, UINT h)
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

void res_load()
{
    const unsigned char white[] = {255, 255, 255, 255};
    res_texWhite = textureFromData(white, 1, 1);
}
