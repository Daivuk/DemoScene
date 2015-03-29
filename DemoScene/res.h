#pragma once
#include <d3d11.h>

extern ID3D11ShaderResourceView* res_texWhite;
extern ID3D11ShaderResourceView** res_textures;
extern int res_textureCount;

void res_load();
