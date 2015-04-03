#include "resTexture.h"

void resTexture::generate()
{
    if (!pTexture)
    {
        delete pTexture;
        pTexture = nullptr;
    }

    auto pData = new uint8_t[w * h * 4];
    memset(pData, 0, w * h * 4);

    pTexture = Texture::createFromData({w, h}, pData);
}
