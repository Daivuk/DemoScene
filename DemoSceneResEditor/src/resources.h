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
    virtual sTextureCmd* copy() = 0;
    struct sTexture* texture = nullptr;
    virtual eRES_CMD getType() = 0;
};

struct sTextureCmdFILL : public sTextureCmd
{
    Color color = Color::White;
    sTextureCmd* copy() override
    {
        auto cmd = new sTextureCmdFILL();
        cmd->color = color;
        return cmd;
    }
    eRES_CMD getType() override { return RES_FILL; }
};

struct sTextureCmdRECT : public sTextureCmd
{
    Color color = Color::White;
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    sTextureCmd* copy() override
    {
        auto cmd = new sTextureCmdRECT();
        cmd->color = color;
        cmd->x1 = x1;
        cmd->y1 = y1;
        cmd->x2 = x2;
        cmd->y2 = y2;
        return cmd;
    }
    eRES_CMD getType() override { return RES_RECT; }
};

struct sTextureCmdBEVEL : public sTextureCmd
{
    Color color = Color::Black;
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    int bevel = 5;
    sTextureCmd* copy() override
    {
        auto cmd = new sTextureCmdBEVEL();
        cmd->color = color;
        cmd->x1 = x1;
        cmd->y1 = y1;
        cmd->x2 = x2;
        cmd->y2 = y2;
        cmd->bevel = bevel;
        return cmd;
    }
    eRES_CMD getType() override { return RES_BEVEL; }
};

struct sTextureCmdCIRCLE : public sTextureCmd
{
    Color color = Color::White;
    int x = 0, y = 0;
    int radius = 20;
    sTextureCmd* copy() override
    {
        auto cmd = new sTextureCmdCIRCLE();
        cmd->color = color;
        cmd->x = x;
        cmd->y = y;
        cmd->radius = radius;
        return cmd;
    }
    eRES_CMD getType() override { return RES_CIRCLE; }
};

struct sTextureCmdBEVEL_CIRCLE : public sTextureCmd
{
    Color color = Color::White;
    int x = 0, y = 0;
    int radius = 20;
    int bevel = 3;
    sTextureCmd* copy() override
    {
        auto cmd = new sTextureCmdBEVEL_CIRCLE();
        cmd->color = color;
        cmd->x = x;
        cmd->y = y;
        cmd->radius = radius;
        cmd->bevel = bevel;
        return cmd;
    }
    eRES_CMD getType() override { return RES_BEVEL_CIRCLE; }
};

struct sTextureCmdLINE : public sTextureCmd
{
    Color color = Color::White;
    int x1 = 8, y1 = 8, x2 = 48, y2 = 8;
    int size = 3;
    sTextureCmd* copy() override
    {
        auto cmd = new sTextureCmdLINE();
        cmd->color = color;
        cmd->x1 = x1;
        cmd->y1 = y1;
        cmd->x2 = x2;
        cmd->y2 = y2;
        cmd->size = size;
        return cmd;
    }
    eRES_CMD getType() override { return RES_LINE; }
};

struct sTextureCmdGRADIENT : public sTextureCmd
{
    Color color1 = Color::White, color2 = Color::Black;
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    bool bVertical = false;
    sTextureCmd* copy() override
    {
        auto cmd = new sTextureCmdGRADIENT();
        cmd->color1 = color1;
        cmd->color2 = color2;
        cmd->x1 = x1;
        cmd->y1 = y1;
        cmd->x2 = x2;
        cmd->y2 = y2;
        cmd->bVertical = bVertical;
        return cmd;
    }
    eRES_CMD getType() override { return RES_GRADIENT; }
};

struct sTextureCmdNORMAL_MAP : public sTextureCmd
{
    sTextureCmd* copy() override
    {
        return new sTextureCmdNORMAL_MAP();
    }
    eRES_CMD getType() override { return RES_NORMAL_MAP; }
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
