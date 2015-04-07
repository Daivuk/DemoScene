#pragma once
#include "onut.h"
#include "img_bake.h"
using namespace std;
using namespace onut;

struct res_palColor;

struct res_Color
{
    uint8_t x, y, z, w;
    res_Color();
    res_Color(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w);
    res_Color(const res_palColor& c);
    ~res_Color();
};

struct res_palColor
{
    uint8_t x, y, z, w;
    res_palColor() {}
    res_palColor(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w);
    res_palColor(const res_Color& c);
};

int res_getColorId(const res_Color& in_color);

struct sTextureCmd
{
    virtual ~sTextureCmd() {}
    virtual sTextureCmd* copy() = 0;
    struct sTexture* texture = nullptr;
    virtual eRES_CMD getType() = 0;
    virtual void serialize() = 0;
    virtual void deserialize() = 0;
};

struct sTextureCmdFILL : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_FILL; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdRECT : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_RECT; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdBEVEL : public sTextureCmd
{
    res_Color color = {0, 0, 0, 255};
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    int bevel = 5;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_BEVEL; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdCIRCLE : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    int x = 0, y = 0;
    int radius = 20;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_CIRCLE; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdBEVEL_CIRCLE : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    int x = 0, y = 0;
    int radius = 20;
    int bevel = 3;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_BEVEL_CIRCLE; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdLINE : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    int x1 = 8, y1 = 8, x2 = 48, y2 = 8;
    int size = 3;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_LINE; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdGRADIENT : public sTextureCmd
{
    res_Color color1 = {255, 255, 255, 255}, color2 = {0, 0, 0, 255};
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    bool bVertical = false;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_GRADIENT; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdNORMAL_MAP : public sTextureCmd
{
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_NORMAL_MAP; }
    void serialize() override;
    void deserialize() override;
};

struct sTextureCmdIMAGE : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    int imgId = 0;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_IMAGE; }
    void serialize() override;
    void deserialize() override;
};

struct sTexture
{
    virtual ~sTexture();
    void bake();
    void serialize();
    void deserialize();
    sTexture* copy() const;

    Texture* texture = nullptr;
    Texture* texNormalMap = nullptr;
    Texture* textMaterialMap = nullptr;

    int w = 256, h = 256;
    vector<sTextureCmd*> cmds;
    uint32_t* data = nullptr;
};

extern vector<sTexture*> res_textures;
extern vector<res_palColor> res_palette;
