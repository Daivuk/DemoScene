#pragma once

extern float gfxData[];

#define RESOLUTION_W 1280
#define RESOLUTION_H 720

#define GFX_VIEWPROJ2D (gfxData)
#define GFX_PROJ3D (gfxData + 34)
#define GFX_IDENTITY (gfxData + 15)
#define GFX_BLACK (gfxData + 27)
#define GFX_WHITE (gfxData + 30)
#define GFX_Z (gfxData + 13)
#define GFX_Y (gfxData + 14)
#define GFX_X (gfxData + 15)
#define GFX_V3_ZERO (gfxData + 16)

void gfx_init();
void gfx_beginFrame();
void gfx_endFrame();
void gfx_setup2d();
void gfx_setup3d(struct sCamera* pActiveCamera);
void gfx_drawModel(struct sModel& model);
