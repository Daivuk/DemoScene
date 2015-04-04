#pragma once
#include "onut.h"
#include "img_bake.h"
using namespace std;
using namespace onut;

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

struct sTextureCmdIMAGE : public sTextureCmd
{
    Color color = Color::White;
    int x1 = 0, y1 = 0, x2 = 32, y2 = 32;
    int imgId = 0;
    sTextureCmd* copy() override;
    eRES_CMD getType() override { return RES_IMAGE; }
    void serialize(vector<uint8_t>& data) override;
    int deserialize(uint8_t* pData) override;
};

struct sTexture
{
    virtual ~sTexture();
    void bake();
    void serialize(vector<uint8_t>& data);
    int deserialize(uint8_t* pData);
    sTexture* copy() const;

    Texture* texture = nullptr;
    Texture* texNormalMap = nullptr;
    Texture* textMaterialMap = nullptr;

    int w = 256, h = 256;
    vector<sTextureCmd*> cmds;
    uint32_t* data = nullptr;
};

extern vector<sTexture*> res_textures;
