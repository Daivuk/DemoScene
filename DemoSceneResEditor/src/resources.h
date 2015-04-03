#pragma once
#include "onut.h"
using namespace std;
using namespace onut;

enum eRES_CMD : uint8_t
{
    RES_IMG,
    RES_FILL,
    RES_RECT,
    RES_BEVEL,
    RES_CIRCLE,
    RES_BEVEL_CIRCLE,
    RES_LINE,
    RES_GRADIENT,
    RES_NORMAL_MAP,
    RES_IMG_END,

    RES_MESH,
    RES_QUAD,
    RES_MESH_END,

    RES_MODEL,
    RES_CAMERA,
    RES_EMITTER,
    RES_LIGHT,
    RES_SPOT_LIGHT,
    RES_SUN_LIGHT,
    RES_AMBIENT
};

struct sTextureCmd
{
    virtual ~sTextureCmd() {}
    struct sTexture* texture = nullptr;
};

struct sTextureCmdFILL : public sTextureCmd
{
    Color color = Color::White;
};

struct sTextureCmdRECT : public sTextureCmd
{
    Color color = Color::White;
    int x = 0, y = 0, w = 16, h = 16;
};

struct sTextureCmdBEVEL : public sTextureCmd
{
    Color color = Color::Black;
    int x = 0, y = 0, w = 16, h = 16;
    int bevel;
};

struct sTextureCmdCIRCLE : public sTextureCmd
{
    Color color = Color::White;
    int x = 0, y = 0;
    int radius;
};

struct sTextureCmdBEVEL_CIRCLE : public sTextureCmd
{
    Color color = Color::White;
    int x = 0, y = 0;
    int radius = 20;
    int bevel = 3;
};

struct sTextureCmdLINE : public sTextureCmd
{
    Color color = Color::White;
    int x1 = 0, y1 = 8, x2 = 16, y2 = 8;
    int size = 3;
};

struct sTextureCmdGRADIENT : public sTextureCmd
{
    Color color1 = Color::White, color2 = Color::Black;
    int x = 0, y = 0, w = 16, h = 16;
    bool bVertical;
};

struct sTextureCmdNORMAL_MAP : public sTextureCmd
{
};

struct sTexture
{
    virtual ~sTexture();
    void bake();

    Texture* texture = nullptr;
    int w = 256, h = 256;
    vector<sTextureCmd*> cmds;
    uint32_t* data = nullptr;
};

extern vector<sTexture*> res_textures;
