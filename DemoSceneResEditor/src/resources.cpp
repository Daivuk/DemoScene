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
