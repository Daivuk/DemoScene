#include "resources.h"
#include "compress.h"

vector<sTexture*> res_textures;
vector<res_palColor> res_palette;

res_palColor::res_palColor(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w)
    : x(_x), y(_y), z(_z), w(_w)
{
}

res_palColor::res_palColor(const res_Color& c)
    : x(c.x), y(c.y), z(c.z), w(c.w)
{
}

int res_getColorId(const res_Color& in_color)
{
    int id = 0;
    for (auto& color : res_palette)
    {
        if (color.x == in_color.x &&
            color.y == in_color.y &&
            color.z == in_color.z &&
            color.w == in_color.w) return id;
        ++id;
    }
    // We add a new color.
    res_palette.push_back(in_color);
    return (int)(res_palette.size() - 1);
}

vector<res_Color*>* allColors = nullptr;

res_Color::res_Color()
{
    if (!allColors) allColors = new vector<res_Color*>();
    allColors->push_back(this);
}

res_Color::res_Color(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w)
    : x(_x), y(_y), z(_z), w(_w)
{
    if (!allColors) allColors = new vector<res_Color*>();
    allColors->push_back(this);
}

res_Color::res_Color(const res_palColor& c)
    : x(c.x), y(c.y), z(c.z), w(c.w)
{
    if (!allColors) allColors = new vector<res_Color*>();
    allColors->push_back(this);
}

res_Color::~res_Color()
{
    for (auto it = allColors->begin(); it != allColors->end(); ++it)
    {
        if (*it == this)
        {
            allColors->erase(it);
            break;
        }
    }
}

sTexture::sTexture()
{
    texture[CHANNEL_DIFFUSE] = nullptr;
    texture[CHANNEL_NORMAL] = nullptr;
    texture[CHANNEL_MATERIAL] = nullptr;
    data[CHANNEL_DIFFUSE] = nullptr;
    data[CHANNEL_NORMAL] = nullptr;
    data[CHANNEL_MATERIAL] = nullptr;
}

sTexture::~sTexture()
{
    if (data[CHANNEL_DIFFUSE]) delete[] data[CHANNEL_DIFFUSE];
    if (data[CHANNEL_NORMAL]) delete[] data[CHANNEL_NORMAL];
    if (data[CHANNEL_MATERIAL]) delete[] data[CHANNEL_MATERIAL];
    if (texture[CHANNEL_DIFFUSE]) delete texture[CHANNEL_DIFFUSE];
    if (texture[CHANNEL_NORMAL]) delete texture[CHANNEL_NORMAL];
    if (texture[CHANNEL_MATERIAL]) delete texture[CHANNEL_MATERIAL];
    for (auto cmd : cmds) delete cmd;
}

uint32_t packColor(const res_Color& color)
{
    return
         ((int)color.x & 0xff) |
        (((int)color.y << 8) & 0xff00) |
        (((int)color.z << 16) & 0xff0000) |
        (((int)color.w << 24) & 0xff000000);
}

void sTexture::bake()
{
    // Clean
    for (int i = 0; i < 3; ++i)
    {
        if (data[i]) delete[] data[i];
        if (texture[i]) delete texture[i];
        data[i] = nullptr;
        texture[i] = nullptr;
    }
    if (!w || !h) return;

    // Allocate
    for (int i = 0; i < 3; ++i)
    {
        data[i] = new uint32_t[w * h];
        ZeroMemory(data[i], h * w * 4);
    }

    if (!cmds.empty())
    {
        // Set our context
        for (int i = 0; i < 3; ++i)
        {
            img.pData[i] = data[i];
        }
        img.w = w;
        img.h = h;

        // Apply commands
        for (auto cmd : cmds)
        {
            cmd->prepare();
            if (dynamic_cast<sTextureCmdFILL*>(cmd))
            {
                auto pCmd = (sTextureCmdFILL*)cmd;
                fill(packColor(pCmd->color));
            }
            else if (dynamic_cast<sTextureCmdLINE*>(cmd))
            {
                auto pCmd = (sTextureCmdLINE*)cmd;
                drawLine(pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2, packColor(pCmd->color), pCmd->size, true);
            }
            else if (dynamic_cast<sTextureCmdRECT*>(cmd))
            {
                auto pCmd = (sTextureCmdRECT*)cmd;
                fillRect(packColor(pCmd->color), pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2);
            }
            else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
            {
                auto pCmd = (sTextureCmdCIRCLE*)cmd;
                drawCircle(pCmd->x, pCmd->y, pCmd->radius, packColor(pCmd->color));
            }
            else if (dynamic_cast<sTextureCmdIMAGE*>(cmd))
            {
                auto pCmd = (sTextureCmdIMAGE*)cmd;
                if (pCmd->imgId < (int)res_textures.size())
                {
                    auto srcTex = res_textures[pCmd->imgId];
                    putImg(packColor(pCmd->color), pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2, 
                           srcTex->data[0],
                           srcTex->data[1],
                           srcTex->data[2],
                           srcTex->w, srcTex->h);
                }
            }
        }
    }

    // Create our final textures
    for (int i = 0; i < 3; ++i)
    {
        if (i == CHANNEL_NORMAL)
        {
            auto normalData = new uint32_t[w * h];
            memcpy(normalData, data[CHANNEL_NORMAL], w * h * 4);
            auto prev = img.pData[CHANNEL_NORMAL];
            img.pData[CHANNEL_NORMAL] = normalData;
            normalMap();
            img.pData[CHANNEL_NORMAL] = prev;
            texture[CHANNEL_NORMAL] = Texture::createFromData({w, h}, (uint8_t*)normalData);
            delete[] normalData;
        }
        else
        {
            texture[i] = Texture::createFromData({w, h}, (uint8_t*)data[i]);
        }
    }
}

sTextureCmd* sTextureCmdFILL::copy()
{
    auto cmd = new sTextureCmdFILL();
    cmd->color = color;

    cmd->bevel = bevel;
    cmd->raise = raise;
    cmd->specular = specular;
    cmd->shininess = shininess;
    cmd->selfIllum = selfIllum;
    cmd->bInvBevel = bInvBevel;
    return cmd;
}

sTextureCmd* sTextureCmdRECT::copy()
{
    auto cmd = new sTextureCmdRECT();
    cmd->color = color;
    cmd->x1 = x1;
    cmd->y1 = y1;
    cmd->x2 = x2;
    cmd->y2 = y2;

    cmd->bevel = bevel;
    cmd->raise = raise;
    cmd->specular = specular;
    cmd->shininess = shininess;
    cmd->selfIllum = selfIllum;
    cmd->bInvBevel = bInvBevel;
    return cmd;
}

sTextureCmd* sTextureCmdCIRCLE::copy()
{
    auto cmd = new sTextureCmdCIRCLE();
    cmd->color = color;
    cmd->x = x;
    cmd->y = y;
    cmd->radius = radius;

    cmd->bevel = bevel;
    cmd->raise = raise;
    cmd->specular = specular;
    cmd->shininess = shininess;
    cmd->selfIllum = selfIllum;
    cmd->bInvBevel = bInvBevel;
    return cmd;
}

sTextureCmd* sTextureCmdLINE::copy()
{
    auto cmd = new sTextureCmdLINE();
    cmd->color = color;
    cmd->x1 = x1;
    cmd->y1 = y1;
    cmd->x2 = x2;
    cmd->y2 = y2;
    cmd->size = size;

    cmd->bevel = bevel;
    cmd->raise = raise;
    cmd->specular = specular;
    cmd->shininess = shininess;
    cmd->selfIllum = selfIllum;
    cmd->bInvBevel = bInvBevel;
    return cmd;
}

sTextureCmd* sTextureCmdIMAGE::copy()
{
    auto cmd = new sTextureCmdIMAGE();
    cmd->color = color;
    cmd->x1 = x1;
    cmd->y1 = y1;
    cmd->x2 = x2;
    cmd->y2 = y2;
    cmd->imgId = imgId;

    cmd->bevel = bevel;
    cmd->raise = raise;
    cmd->specular = specular;
    cmd->shininess = shininess;
    cmd->selfIllum = selfIllum;
    cmd->bInvBevel = bInvBevel;
    return cmd;
}

uint8_t packPos(int pos)
{
    return (pos + 32) / 4;
}

void sTextureCmdFILL::serialize()
{
    write(RES_FILL, 8);
    write(res_getColorId(color), 8);
}

void sTextureCmdFILL::deserialize()
{
    color = res_palette[readBits(8)];
}

void sTextureCmdRECT::serialize()
{
    write(RES_RECT, 8);
    write(res_getColorId(color), 8);

    write((int)packPos(x1), 8);
    write((int)packPos(y1), 8);
    write((int)packPos(x2), 8);
    write((int)packPos(y2), 8);
}

void sTextureCmdRECT::deserialize()
{
    color = res_palette[readBits(8)];

    x1 = unpackPos(readBits(8));
    y1 = unpackPos(readBits(8));
    x2 = unpackPos(readBits(8));
    y2 = unpackPos(readBits(8));
}

void sTextureCmdCIRCLE::serialize()
{
    write(RES_CIRCLE, 8);
    write(res_getColorId(color), 8);

    write((int)packPos(x), 8);
    write((int)packPos(y), 8);

    write(radius - 1, 8);
}

void sTextureCmdCIRCLE::deserialize()
{
    color = res_palette[readBits(8)];

    x = unpackPos(readBits(8));
    y = unpackPos(readBits(8));

    radius = readBits(8) + 1;
}

void sTextureCmdLINE::serialize()
{
    write(RES_LINE, 8);
    write(res_getColorId(color), 8);

    write((int)packPos(x1), 8);
    write((int)packPos(y1), 8);
    write((int)packPos(x2), 8);
    write((int)packPos(y2), 8);

    write(size - 1, 6);
}

void sTextureCmdLINE::deserialize()
{
    color = res_palette[readBits(8)];

    x1 = unpackPos(readBits(8));
    y1 = unpackPos(readBits(8));
    x2 = unpackPos(readBits(8));
    y2 = unpackPos(readBits(8));

    size = readBits(6) + 1;
}

void sTextureCmdIMAGE::serialize()
{
    write(RES_IMAGE, 8);
    write(res_getColorId(color), 8);

    write((int)packPos(x1), 8);
    write((int)packPos(y1), 8);
    write((int)packPos(x2), 8);
    write((int)packPos(y2), 8);

    write(imgId, 8);
}

void sTextureCmdIMAGE::deserialize()
{
    color = res_palette[readBits(8)];

    x1 = unpackPos(readBits(8));
    y1 = unpackPos(readBits(8));
    x2 = unpackPos(readBits(8));
    y2 = unpackPos(readBits(8));

    imgId = readBits(8);
}

void sTextureCmd::prepare()
{
    img.bakeState.bevel = bevel;
    img.bakeState.raise = raise;
    img.bakeState.specular = specular;
    img.bakeState.shininess = shininess;
    img.bakeState.selfIllum = selfIllum;
}

int findExpo(int texSize)
{
    int expo = 0;
    while (texSize > 1)
    {
        texSize /= 2;;
        expo++;
    }
    return expo;
}

int imgDimToBits(int texSize)
{
    return findExpo(texSize) - 3;
}

void sTexture::serialize()
{
    write(RES_IMG, 8);
    write(imgDimToBits(w), 3);
    write(imgDimToBits(h), 3);
    for (auto cmd : cmds)
    {
        cmd->serialize();
    }
    write(RES_IMG_END, 8);
}

void sTexture::deserialize()
{
    w = readBits(3);
    h = readBits(3);

    w = (int)std::pow<int, int>(2, w + 3);
    h = (int)std::pow<int, int>(2, h + 3);

    bool hasNormalMap = readBits(1) ? true : false;
    bool hasMaterialMap = readBits(1) ? true : false;

    auto b = (uint8_t)readBits(8);
    while (b != RES_IMG_END)
    {
        sTextureCmd* cmd;
        switch (b)
        {
            case RES_FILL: cmd = new sTextureCmdFILL(); break;
            case RES_LINE: cmd = new sTextureCmdLINE(); break;
            case RES_RECT: cmd = new sTextureCmdRECT(); break;
            case RES_CIRCLE: cmd = new sTextureCmdCIRCLE(); break;
            case RES_IMAGE: cmd = new sTextureCmdIMAGE(); break;
        }
        cmd->deserialize();
        cmds.push_back(cmd);
        b = (uint8_t)readBits(8);
    }
}

sTexture* sTexture::copy() const
{
    sTexture* pRet = new sTexture();

    pRet->w = w;
    pRet->h = h;
    
    for (auto cmd : cmds) pRet->cmds.push_back(cmd->copy());

    pRet->bake();
    return pRet;
}
