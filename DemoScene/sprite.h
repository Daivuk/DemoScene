#pragma once
#include <d3d11.h>

void spr_init();
void spr_draw(ID3D11ShaderResourceView* pTexture, const float* rect, const float* color);
void spr_flush();
