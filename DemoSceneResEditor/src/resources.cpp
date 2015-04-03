#include "resources.h"

vector<sTexture*> res_textures;

sTexture::~sTexture()
{
    if (data) delete[] data;
    if (texture) delete texture;
}

void sTexture::bake()
{
    if (data) delete[] data;
    if (texture) delete texture;
    data = nullptr;
    texture = nullptr;
    if (!w || !h) return;

    data = new uint32_t[w * h];

    texture = Texture::createFromData({w, h}, (uint8_t*)data);
}
