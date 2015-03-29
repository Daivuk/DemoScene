#pragma once

extern float gfxData[];

#define GFX_VIEWPROJ2D (gfxData)
#define GFX_BLACK (gfxData + 16)
#define GFX_WHITE (gfxData + 19)

void gfx_init();
void gfx_beginFrame();
void gfx_endFrame();
void gfx_setup2d();
