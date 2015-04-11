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
    struct sTexture* texture = nullptr;
    int bevel = 0;
    int raise = 0;
    int specular = 0;
    int shininess = 0;
    int selfIllum = 0;
    virtual ~sTextureCmd() {}
    virtual sTextureCmd* copy() = 0;
    virtual eRES_CMD getType() = 0;
    virtual void serialize() = 0;
    virtual void deserialize() = 0;
    virtual void prepare();
};

struct sTextureCmdFILL : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_FILL; }
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

struct sTextureCmdRECT : public sTextureCmd
{
    res_Color color = {255, 255, 255, 255};
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_RECT; }
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

enum eTextureChannel
{
    CHANNEL_DIFFUSE = 0,
    CHANNEL_NORMAL = 1,
    CHANNEL_MATERIAL = 2
};

struct sTexture
{
    sTexture();
    virtual ~sTexture();
    void bake();
    void serialize();
    void deserialize();
    sTexture* copy() const;

    Texture* texture[3];

    int w = 256, h = 256;
    vector<sTextureCmd*> cmds;
    uint32_t* data[3];
};

extern vector<sTexture*> res_textures;
extern vector<res_palColor> res_palette;
