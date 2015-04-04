#include "resources.h"
#include "img_bake.h"

vector<sTexture*> res_textures;

sTexture::~sTexture()
{
    if (data) delete[] data;
    if (texture) delete texture;
}

uint32_t packColor(const Color& color)
{
    return
        (((int)(color.x * 255.f)) & 0xff) |
        (((int)(color.y * 255.f) << 8) & 0xff00) |
        (((int)(color.z * 255.f) << 16) & 0xff0000) |
        (((int)(color.w * 255.f) << 24) & 0xff000000);
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
                sTextureCmdFILL* pCmd = (sTextureCmdFILL*)cmd;
                fill(packColor(pCmd->color));
            }
            else if (dynamic_cast<sTextureCmdRECT*>(cmd))
            {
                sTextureCmdRECT* pCmd = (sTextureCmdRECT*)cmd;
                fillRect(packColor(pCmd->color), pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2);
            }
            else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
            {
                sTextureCmdBEVEL* pCmd = (sTextureCmdBEVEL*)cmd;
                bevel(packColor(pCmd->color), pCmd->bevel, pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2);
            }
            else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
            {
                sTextureCmdCIRCLE* pCmd = (sTextureCmdCIRCLE*)cmd;
                drawCircle(pCmd->x, pCmd->y, pCmd->radius, packColor(pCmd->color));
            }
            else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
            {
                sTextureCmdBEVEL_CIRCLE* pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;
                drawCircle(pCmd->x, pCmd->y, pCmd->radius, packColor(pCmd->color), pCmd->bevel);
            }
            else if (dynamic_cast<sTextureCmdLINE*>(cmd))
            {
                sTextureCmdLINE* pCmd = (sTextureCmdLINE*)cmd;
                drawLine(pCmd->x1, pCmd->y1, pCmd->x2, pCmd->y2, packColor(pCmd->color), pCmd->size);
            }
            else if (dynamic_cast<sTextureCmdNORMAL_MAP*>(cmd))
            {
                sTextureCmdNORMAL_MAP* pCmd = (sTextureCmdNORMAL_MAP*)cmd;
                normalMap();
            }
            else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
            {
                sTextureCmdGRADIENT* pCmd = (sTextureCmdGRADIENT*)cmd;
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

uint8_t packPos(int pos)
{
    return (pos + 32) / 4;
}

int unpackPos(uint8_t pos)
{
    return (int)pos * 4 - 32;
}

void sTextureCmdFILL::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_FILL);

    data.push_back((uint8_t)(color.x * 255.f));
    data.push_back((uint8_t)(color.y * 255.f));
    data.push_back((uint8_t)(color.z * 255.f));
    data.push_back((uint8_t)(color.w * 255.f));
}

int sTextureCmdFILL::deserialize(uint8_t* pData)
{
    color.x = (float)pData[0] / 255.f;
    color.y = (float)pData[1] / 255.f;
    color.z = (float)pData[2] / 255.f;
    color.w = (float)pData[3] / 255.f;

    return 4;
}

void sTextureCmdRECT::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_RECT);

    data.push_back((uint8_t)(color.x * 255.f));
    data.push_back((uint8_t)(color.y * 255.f));
    data.push_back((uint8_t)(color.z * 255.f));
    data.push_back((uint8_t)(color.w * 255.f));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));
}

int sTextureCmdRECT::deserialize(uint8_t* pData)
{
    color.x = (float)pData[0] / 255.f;
    color.y = (float)pData[1] / 255.f;
    color.z = (float)pData[2] / 255.f;
    color.w = (float)pData[3] / 255.f;

    x1 = unpackPos(pData[4]);
    y1 = unpackPos(pData[5]);
    x2 = unpackPos(pData[6]);
    y2 = unpackPos(pData[7]);

    return 8;
}

void sTextureCmdBEVEL::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_BEVEL);

    data.push_back((uint8_t)(color.x * 255.f));
    data.push_back((uint8_t)(color.y * 255.f));
    data.push_back((uint8_t)(color.z * 255.f));
    data.push_back((uint8_t)(color.w * 255.f));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));

    data.push_back((uint8_t)bevel);
}

int sTextureCmdBEVEL::deserialize(uint8_t* pData)
{
    color.x = (float)pData[0] / 255.f;
    color.y = (float)pData[1] / 255.f;
    color.z = (float)pData[2] / 255.f;
    color.w = (float)pData[3] / 255.f;

    x1 = unpackPos(pData[4]);
    y1 = unpackPos(pData[5]);
    x2 = unpackPos(pData[6]);
    y2 = unpackPos(pData[7]);

    bevel = (int)pData[8];

    return 9;
}

void sTextureCmdCIRCLE::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_CIRCLE);

    data.push_back((uint8_t)(color.x * 255.f));
    data.push_back((uint8_t)(color.y * 255.f));
    data.push_back((uint8_t)(color.z * 255.f));
    data.push_back((uint8_t)(color.w * 255.f));

    data.push_back(packPos(x));
    data.push_back(packPos(y));

    data.push_back((uint8_t)radius);
}

int sTextureCmdCIRCLE::deserialize(uint8_t* pData)
{
    color.x = (float)pData[0] / 255.f;
    color.y = (float)pData[1] / 255.f;
    color.z = (float)pData[2] / 255.f;
    color.w = (float)pData[3] / 255.f;

    x = unpackPos(pData[4]);
    y = unpackPos(pData[5]);

    radius = (int)pData[6];

    return 7;
}

void sTextureCmdBEVEL_CIRCLE::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_BEVEL_CIRCLE);

    data.push_back((uint8_t)(color.x * 255.f));
    data.push_back((uint8_t)(color.y * 255.f));
    data.push_back((uint8_t)(color.z * 255.f));
    data.push_back((uint8_t)(color.w * 255.f));

    data.push_back(packPos(x));
    data.push_back(packPos(y));

    data.push_back((uint8_t)radius);
    data.push_back((uint8_t)bevel);
}

int sTextureCmdBEVEL_CIRCLE::deserialize(uint8_t* pData)
{
    color.x = (float)pData[0] / 255.f;
    color.y = (float)pData[1] / 255.f;
    color.z = (float)pData[2] / 255.f;
    color.w = (float)pData[3] / 255.f;

    x = unpackPos(pData[4]);
    y = unpackPos(pData[5]);

    radius = (int)pData[6];
    bevel = (int)pData[7];

    return 8;
}

void sTextureCmdLINE::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_LINE);

    data.push_back((uint8_t)(color.x * 255.f));
    data.push_back((uint8_t)(color.y * 255.f));
    data.push_back((uint8_t)(color.z * 255.f));
    data.push_back((uint8_t)(color.w * 255.f));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));

    data.push_back((uint8_t)size);
}

int sTextureCmdLINE::deserialize(uint8_t* pData)
{
    color.x = (float)pData[0] / 255.f;
    color.y = (float)pData[1] / 255.f;
    color.z = (float)pData[2] / 255.f;
    color.w = (float)pData[3] / 255.f;

    x1 = unpackPos(pData[4]);
    y1 = unpackPos(pData[5]);
    x2 = unpackPos(pData[6]);
    y2 = unpackPos(pData[7]);

    size = (int)pData[8];

    return 9;
}

void sTextureCmdGRADIENT::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_GRADIENT);

    data.push_back((uint8_t)(color1.x * 255.f));
    data.push_back((uint8_t)(color1.y * 255.f));
    data.push_back((uint8_t)(color1.z * 255.f));
    data.push_back((uint8_t)(color1.w * 255.f));

    data.push_back((uint8_t)(color2.x * 255.f));
    data.push_back((uint8_t)(color2.y * 255.f));
    data.push_back((uint8_t)(color2.z * 255.f));
    data.push_back((uint8_t)(color2.w * 255.f));

    data.push_back(packPos(x1));
    data.push_back(packPos(y1));
    data.push_back(packPos(x2));
    data.push_back(packPos(y2));

    data.push_back(bVertical ? 1 : 0);
}

int sTextureCmdGRADIENT::deserialize(uint8_t* pData)
{
    color1.x = (float)pData[0] / 255.f;
    color1.y = (float)pData[1] / 255.f;
    color1.z = (float)pData[2] / 255.f;
    color1.w = (float)pData[3] / 255.f;

    color2.x = (float)pData[4] / 255.f;
    color2.y = (float)pData[5] / 255.f;
    color2.z = (float)pData[6] / 255.f;
    color2.w = (float)pData[7] / 255.f;

    x1 = unpackPos(pData[8]);
    y1 = unpackPos(pData[9]);
    x2 = unpackPos(pData[10]);
    y2 = unpackPos(pData[11]);

    bVertical = pData[12] ? true : false;

    return 13;
}

void sTextureCmdNORMAL_MAP::serialize(vector<uint8_t>& data)
{
    data.push_back(RES_NORMAL_MAP);
}

int sTextureCmdNORMAL_MAP::deserialize(uint8_t* pData)
{
    return 0;
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
        }
        size += cmd->deserialize(pData + size + 1);
        cmds.push_back(cmd);

        size++;
    }

    return size;
}
