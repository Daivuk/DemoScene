#pragma once

extern float gfxData[];

#define GFX_VIEWPROJ2D (gfxData)
#define GFX_PROJ3D (gfxData)
#define GFX_IDENTITY (gfxData + 15)
#define GFX_BLACK (gfxData + 27)
#define GFX_WHITE (gfxData + 30)

void gfx_init();
void gfx_beginFrame();
void gfx_endFrame();
void gfx_setup2d();
void gfx_setup3d();
void gfx_drawModel(struct sModel& model);
