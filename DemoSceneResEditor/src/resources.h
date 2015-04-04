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
    virtual void serialize(vector<uint8_t>& data) = 0;
    virtual int deserialize(uint8_t* pData) = 0;
};

struct sTextureCmdFILL : public sTextureCmd
{
    Color color = Color::White;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_FILL; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTextureCmdRECT : public sTextureCmd
{
    Color color = Color::White;
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_RECT; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTextureCmdBEVEL : public sTextureCmd
{
    Color color = Color::Black;
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    int bevel = 5;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_BEVEL; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTextureCmdCIRCLE : public sTextureCmd
{
    Color color = Color::White;
    int x = 0, y = 0;
    int radius = 20;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_CIRCLE; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTextureCmdBEVEL_CIRCLE : public sTextureCmd
{
    Color color = Color::White;
    int x = 0, y = 0;
    int radius = 20;
    int bevel = 3;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_BEVEL_CIRCLE; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTextureCmdLINE : public sTextureCmd
{
    Color color = Color::White;
    int x1 = 8, y1 = 8, x2 = 48, y2 = 8;
    int size = 3;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_LINE; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTextureCmdGRADIENT : public sTextureCmd
{
    Color color1 = Color::White, color2 = Color::Black;
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    bool bVertical = false;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_GRADIENT; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTextureCmdNORMAL_MAP : public sTextureCmd
{
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_NORMAL_MAP; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTexture
{
    virtual ~sTexture();
    void bake();
    void serialize(vector<uint8_t>& data);
    int deserialize(uint8_t* pData);

    Texture* texture = nullptr;
    Texture* texNormalMap = nullptr;
    Texture* textMaterialMap = nullptr;

    int w = 256, h = 256;
    vector<sTextureCmd*> cmds;
    uint32_t* data = nullptr;
};

extern vector<sTexture*> res_textures;
