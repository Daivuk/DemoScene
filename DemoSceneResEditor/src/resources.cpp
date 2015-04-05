#include "resources.h"

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

sTexture::~sTexture()
{
    if (data) delete[] data;
    if (texture) delete texture;
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
    if (data) delete[] data;
    if (texture) delete texture;
    data = nullptr;
    texture = nullptr;
    if (!w || !h) return;

    // Allocate memory
    data = new uint32_t[w * h];
    ZeroMemory(data, h * w * 4);

    if (!cmds.empty())
    {
        // Set our context
        img.pData = data;
        img.w = w;
        img.h = h;

        // Apply commands
        for (auto cmd : cmds)
        {
            if (dynamic_cast<sTextureCmdFILL*>(cmd))
            {
                auto* pCmd = (sTextureCmdFILL*)cmd;
                fill(packColor(pCmd->color));
            }
            else if (dynamic_cast<sTextureCmdRECT*>(cmd))
            {
                auto* pCmd = (sTextureCmdRECT*)cmd;
                fillRect(packColor(pCmd->color), pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2);
            }
            else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
            {
                auto* pCmd = (sTextureCmdBEVEL*)cmd;
                bevel(packColor(pCmd->color), pCmd->bevel, pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2);
            }
            else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
            {
                auto* pCmd = (sTextureCmdCIRCLE*)cmd;
                drawCircle(pCmd->x, pCmd->y, pCmd->radius, packColor(pCmd->color));
            }
            else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
            {
                auto* pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;
                drawCircle(pCmd->x, pCmd->y, pCmd->radius, packColor(pCmd->color), pCmd->bevel);
            }
            else if (dynamic_cast<sTextureCmdLINE*>(cmd))
            {
                auto* pCmd = (sTextureCmdLINE*)cmd;
                drawLine(pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2, packColor(pCmd->color), pCmd->size);
            }
            else if (dynamic_cast<sTextureCmdNORMAL_MAP*>(cmd))
            {
                auto* pCmd = (sTextureCmdNORMAL_MAP*)cmd;
                normalMap();
            }
            else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
            {
                auto* pCmd = (sTextureCmdGRADIENT*)cmd;
            }
            else if (dynamic_cast<sTextureCmdIMAGE*>(cmd))
            {
                auto* pCmd = (sTextureCmdIMAGE*)cmd;
                if (pCmd->imgId < (int)res_textures.size())
                {
                    auto srcTex = res_textures[pCmd->imgId];
                    if (srcTex->data)
                    {
                        putImg(packColor(pCmd->color), pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2, srcTex->data, srcTex->w, srcTex->h);
                    }
                }
            }
        }
    }

    // Create our final texture
    texture = Texture::createFromData({w, h}, (uint8_t*)data);
}

sTextureCmd* sTextureCmdFILL::copy()
{
    auto cmd = new sTextureCmdFILL();
    cmd->color = color;
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
    return cmd;
}

sTextureCmd* sTextureCmdBEVEL::copy()
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

sTextureCmd* sTextureCmdCIRCLE::copy()
{
    auto cmd = new sTextureCmdCIRCLE();
    cmd->color = color;
    cmd->x = x;
    cmd->y = y;
    cmd->radius = radius;
    return cmd;
}

sTextureCmd* sTextureCmdBEVEL_CIRCLE::copy()
{
    auto cmd = new sTextureCmdBEVEL_CIRCLE();
    cmd->color = color;
    cmd->x = x;
    cmd->y = y;
    cmd->radius = radius;
    cmd->bevel = bevel;
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
    return cmd;
}

sTextureCmd* sTextureCmdGRADIENT::copy()
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

sTextureCmd* sTextureCmdNORMAL_MAP::copy()
{
    return new sTextureCmdNORMAL_MAP();
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
    return cmd;
}

uint8_t packPos(int pos)
{
    return (pos + 32) / 4;
}

void sTextureCmdFILL::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_FILL);
    data.push_back((uint8_t)res_getColorId(color));
}

int sTextureCmdFILL::deserialize(uint8_t* pData)
{
    color = res_palette[pData[0]];

    return 1;
}

void sTextureCmdRECT::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_RECT);
    data.push_back((uint8_t)res_getColorId(color));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));
}

int sTextureCmdRECT::deserialize(uint8_t* pData)
{
    color = res_palette[pData[0]];

    x1 = unpackPos(pData[1]);
    y1 = unpackPos(pData[2]);
    x2 = unpackPos(pData[3]);
    y2 = unpackPos(pData[4]);

    return 5;
}

void sTextureCmdBEVEL::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_BEVEL);
    data.push_back((uint8_t)res_getColorId(color));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));

    data.push_back((uint8_t)bevel);
}

int sTextureCmdBEVEL::deserialize(uint8_t* pData)
{
    color = res_palette[pData[0]];

    x1 = unpackPos(pData[1]);
    y1 = unpackPos(pData[2]);
    x2 = unpackPos(pData[3]);
    y2 = unpackPos(pData[4]);

    bevel = (int)pData[5];

    return 6;
}

void sTextureCmdCIRCLE::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_CIRCLE);
    data.push_back((uint8_t)res_getColorId(color));

    data.push_back(packPos(x));
    data.push_back(packPos(y));

    data.push_back((uint8_t)radius);
}

int sTextureCmdCIRCLE::deserialize(uint8_t* pData)
{
    color = res_palette[pData[0]];

    x = unpackPos(pData[1]);
    y = unpackPos(pData[2]);

    radius = (int)pData[3];

    return 4;
}

void sTextureCmdBEVEL_CIRCLE::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_BEVEL_CIRCLE);
    data.push_back((uint8_t)res_getColorId(color));

    data.push_back(packPos(x));
    data.push_back(packPos(y));

    data.push_back((uint8_t)radius);
    data.push_back((uint8_t)bevel);
}

int sTextureCmdBEVEL_CIRCLE::deserialize(uint8_t* pData)
{
    color = res_palette[pData[0]];

    x = unpackPos(pData[1]);
    y = unpackPos(pData[2]);

    radius = (int)pData[3];
    bevel = (int)pData[4];

    return 5;
}

void sTextureCmdLINE::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_LINE);
    data.push_back((uint8_t)res_getColorId(color));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));

    data.push_back((uint8_t)size);
}

int sTextureCmdLINE::deserialize(uint8_t* pData)
{
    color = res_palette[pData[0]];

    x1 = unpackPos(pData[1]);
    y1 = unpackPos(pData[2]);
    x2 = unpackPos(pData[3]);
    y2 = unpackPos(pData[4]);

    size = (int)pData[5];

    return 6;
}

void sTextureCmdGRADIENT::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_GRADIENT);
    data.push_back((uint8_t)res_getColorId(color1));
    data.push_back((uint8_t)res_getColorId(color2));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));

    data.push_back(bVertical ? 1 : 0);
}

int sTextureCmdGRADIENT::deserialize(uint8_t* pData)
{
    color1 = res_palette[pData[0]];
    color2 = res_palette[pData[1]];

    x1 = unpackPos(pData[2]);
    y1 = unpackPos(pData[3]);
    x2 = unpackPos(pData[4]);
    y2 = unpackPos(pData[5]);

    bVertical = pData[6] ? true : false;

    return 7;
}

void sTextureCmdNORMAL_MAP::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_NORMAL_MAP);
}

int sTextureCmdNORMAL_MAP::deserialize(uint8_t* pData)
{
    return 0;
}

void sTextureCmdIMAGE::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_IMAGE);
    data.push_back((uint8_t)res_getColorId(color));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));

    data.push_back((uint8_t)imgId);
}

int sTextureCmdIMAGE::deserialize(uint8_t* pData)
{
    color = res_palette[pData[0]];

    x1 = unpackPos(pData[1]);
    y1 = unpackPos(pData[2]);
    x2 = unpackPos(pData[3]);
    y2 = unpackPos(pData[4]);

    imgId = (int)pData[5];

    return 6;
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

void sTexture::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_IMG);
    data.push_back(findExpo(w));
    data.push_back(findExpo(h));
    for (auto cmd : cmds)
    {
        cmd->serialize(data);
    }
    data.push_back(RES_IMG_END);
}

int sTexture::deserialize(uint8_t* pData)
{
    int size = 2;

    w = (int)std::pow<int, int>(2, (int)pData[0]);
    h = (int)std::pow<int, int>(2, (int)pData[1]);

    while (pData[size] != RES_IMG_END)
    {
        sTextureCmd* cmd;
        switch (pData[size])
        {
            case RES_FILL: cmd = new sTextureCmdFILL(); break;
            case RES_RECT: cmd = new sTextureCmdRECT(); break;
            case RES_BEVEL: cmd = new sTextureCmdBEVEL(); break;
            case RES_CIRCLE: cmd = new sTextureCmdCIRCLE(); break;
            case RES_BEVEL_CIRCLE: cmd = new sTextureCmdBEVEL_CIRCLE(); break;
            case RES_LINE: cmd = new sTextureCmdLINE(); break;
            case RES_GRADIENT: cmd = new sTextureCmdGRADIENT(); break;
            case RES_NORMAL_MAP: cmd = new sTextureCmdNORMAL_MAP(); break;
            case RES_IMAGE: cmd = new sTextureCmdIMAGE(); break;
        }
        size += cmd->deserialize(pData + size + 1);
        cmds.push_back(cmd);

        size++;
    }

    return size;
}

sTexture* sTexture::copy() const
{
    sTexture* pRet = new sTexture();

    pRet->w = w;
    pRet->h = h;
    
    for (auto cmd : cmds)
    {
        pRet->cmds.push_back(cmd->copy());
    }

    pRet->bake();
    return pRet;
}
