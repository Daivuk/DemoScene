#include "MainVC.h"
#include "styles.h"

using namespace std;

static const Color g_guideColor = OColorHex(3399ff);
static const Color g_toolBtnHoverColor = OColorHex(3e3e40);
static const Color g_toolBtnDownColor = OColorHex(007acc);
static const Color g_toolColorSliderBG = OColorHex(000000);

int colorPickerSliders[4];

MainVC::MainVC()
    : uiContext({(float)OSettings->getResolution().x, (float)OSettings->getResolution().y})
    , uiScreen("../../assets/ui/main.json")
{
    // create UI styles
    createUIStyles(&uiContext);

    // create UI styles
    uiContext.addStyle<UICheckBox>("selectBox", [this](const onut::UICheckBox* pCheckBox, const onut::sUIRect& rect)
    {
        auto state = pCheckBox->getState(uiContext);
        auto orect = UI2Onut(rect);
        switch (state)
        {
            case eUIState::HOVER:
                OSB->drawRect(nullptr, orect, g_toolBtnHoverColor);
                break;
            case eUIState::DOWN:
                OSB->drawRect(nullptr, orect, g_toolBtnDownColor);
                break;
        }
        if (pCheckBox->getIsChecked())
        {
            OSB->drawRect(nullptr, orect, g_guideColor);
        }
    });

    uiContext.addStyle<UICheckBox>("selectTexture", [this](const onut::UICheckBox* pCheckBox, const onut::sUIRect& rect)
    {
        auto state = pCheckBox->getState(uiContext);
        auto orect = UI2Onut(rect);
        orect.z -= 4;
        orect.w -= 4;
        switch (state)
        {
            case eUIState::HOVER:
                OSB->drawRect(nullptr, orect, g_toolBtnHoverColor);
                break;
            case eUIState::DOWN:
                OSB->drawRect(nullptr, orect, g_toolBtnDownColor);
                break;
        }
        if (pCheckBox->getIsChecked())
        {
            OSB->drawRect(nullptr, orect, g_guideColor);
        }
        auto resT = (sTexture*)pCheckBox->pUserData;
        orect.x += 2;
        orect.y += 2;
        orect.z -= 4;
        orect.w -= 4;
        OSB->drawRectWithUVs(OGetTexture("dottedLine.png"), orect, {0, 0, orect.z / 16, orect.w / 16}, g_toolBtnHoverColor);
        if (resT->texture)
        {
            OSB->drawRect(resT->texture, orect);
        }
    });

    uiContext.addStyle<UIPanel>("pnlTexture", [this](const onut::UIPanel* pCheckBox, const onut::sUIRect& rect)
    {
        auto state = pCheckBox->getState(uiContext);
        auto orect = UI2Onut(rect);
        OSB->drawRectWithUVs(OGetTexture("dottedLine.png"), orect, {0, 0, orect.z / 16, orect.w / 16}, g_toolBtnHoverColor);
        if (workingTexture->texture)
        {
            OSB->drawRect(workingTexture->texture, orect);
        }
    });

    uiContext.addStyle<onut::UIPanel>("colorSlider", [](const onut::UIPanel* pPanel, const onut::sUIRect& rect)
    {
        auto colorIndex = reinterpret_cast<uintptr_t>(pPanel->pUserData);

        auto orect = onut::UI2Onut(rect);
        OSB->drawRect(nullptr, orect, g_toolColorSliderBG);
        orect.w = (float)colorPickerSliders[colorIndex];
        orect.y += 255 - (float)colorPickerSliders[colorIndex];
        if (colorIndex == 0)
            OSB->drawRect(nullptr, orect, Color(1, 0, 0, 1));
        if (colorIndex == 1)
            OSB->drawRect(nullptr, orect, Color(0, 1, 0, 1));
        if (colorIndex == 2)
            OSB->drawRect(nullptr, orect, Color(0, 0, 1, 1));
        if (colorIndex == 3)
            OSB->drawRect(nullptr, orect, Color(1, 1, 1, 1));
    });

    // Fetch stuff
    uiTextures = uiScreen.getChild("viewTextures");
    uiTexture = uiScreen.getChild("viewTexture");
    uiInspectorTextures = uiScreen.getChild("inspectorTextures");
    uiInspectorTexture = uiScreen.getChild("inspectorTexture");
    uiColorPicker = uiScreen.getChild("dlgColorPicker");
    uiCmdStack = uiInspectorTexture->getChild("pnlCmdStack");
    cmdControls[eRES_CMD::RES_FILL] = uiInspectorTexture->getChild<UICheckBox>("chkCmdFILL");
    cmdControls[eRES_CMD::RES_RECT] = uiInspectorTexture->getChild<UICheckBox>("chkCmdRECT");
    cmdControls[eRES_CMD::RES_BEVEL] = uiInspectorTexture->getChild<UICheckBox>("chkCmdBEVEL");
    cmdControls[eRES_CMD::RES_CIRCLE] = uiInspectorTexture->getChild<UICheckBox>("chkCmdCIRCLE");
    cmdControls[eRES_CMD::RES_BEVEL_CIRCLE] = uiInspectorTexture->getChild<UICheckBox>("chkCmdBEVEL_CIRCLE");
    cmdControls[eRES_CMD::RES_LINE] = uiInspectorTexture->getChild<UICheckBox>("chkCmdLINE");
    cmdControls[eRES_CMD::RES_NORMAL_MAP] = uiInspectorTexture->getChild<UICheckBox>("chkCmdNORMAL_MAP");
    cmdControls[eRES_CMD::RES_GRADIENT] = uiInspectorTexture->getChild<UICheckBox>("chkCmdGRADIENT");
    for (auto& kv : cmdControls)
    {
        kv.second->retain();
        kv.second->behavior = eUICheckBehavior::EXCLUSIVE;
        kv.second->remove();
    }
    pnlTexture = uiTexture->getChild("pnlTexture");
    uiTextureW = uiInspectorTexture->getChild<UITextBox>("txtImgW");
    uiTextureH = uiInspectorTexture->getChild<UITextBox>("txtImgH");
    uiTextureW->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        auto d = uiTextureW->getInt();
        pnlTexture->rect.size.x = (float)d;
        workingTexture->w = d;
    };
    uiTextureH->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        auto d = uiTextureH->getInt();
        pnlTexture->rect.size.y = (float)d;
        workingTexture->h = d;
    };

    // Events
    uiInspectorTexture->getChild("btnCmdFILL")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdFILL(), cmdControls[eRES_CMD::RES_FILL]->copy());
    };
    uiInspectorTexture->getChild("btnCmdRECT")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdRECT(), cmdControls[eRES_CMD::RES_RECT]->copy());
    };
    uiInspectorTexture->getChild("btnCmdBEVEL")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdBEVEL(), cmdControls[eRES_CMD::RES_BEVEL]->copy());
    };
    uiInspectorTexture->getChild("btnCmdCIRCLE")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdCIRCLE(), cmdControls[eRES_CMD::RES_CIRCLE]->copy());
    };
    uiInspectorTexture->getChild("btnCmdBEVEL_CIRCLE")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdBEVEL_CIRCLE(), cmdControls[eRES_CMD::RES_BEVEL_CIRCLE]->copy());
    };
    uiInspectorTexture->getChild("btnCmdLINE")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdLINE(), cmdControls[eRES_CMD::RES_LINE]->copy());
    };
    uiInspectorTexture->getChild("btnCmdNORMAL_MAP")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdNORMAL_MAP(), cmdControls[eRES_CMD::RES_NORMAL_MAP]->copy());
    };
    uiInspectorTexture->getChild("btnCmdGRADIENT")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdGRADIENT(), cmdControls[eRES_CMD::RES_GRADIENT]->copy());
    };
    uiScreen.getChild("btnTextures")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        closeAllViews();
        uiTextures->isVisible = true;
        uiInspectorTextures->isVisible = true;
    };
    uiScreen.getChild("btnMeshes")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        closeAllViews();
    };
    uiScreen.getChild("btnScene")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        closeAllViews();
    };
    uiInspectorTextures->getChild("btnNewTexture")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        auto pTexture = new sTexture();
        auto pSelectBox = new UICheckBox();

        pSelectBox->rect.size = {128, 128};
        pSelectBox->pUserData = pTexture;
        pSelectBox->setStyle("selectTexture");
        pSelectBox->behavior = eUICheckBehavior::EXCLUSIVE;

        auto selected = getSelectedTexture();
        auto index = selected.index;
        if (index < res_textures.size()) ++index;
        uiTextures->insertAfter(pSelectBox, selected.selectBox);
        res_textures.insert(res_textures.begin() + index, pTexture);

        pSelectBox->setIsChecked(true);
    };
    uiInspectorTextures->getChild("btnEditTexture")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        auto selected = getSelectedTexture();
        if (selected.texture)
        {
            closeAllViews();
            workingTexture = selected.texture;
            uiTexture->isVisible = true;
            uiInspectorTexture->isVisible = true;
            buildUIForTexture();
        }
    };
    uiInspectorTextures->getChild("btnDeleteTexture")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        auto selected = getSelectedTexture();
        if (selected.texture)
        {
            uiContext.clearState();
            uiTextures->remove(selected.selectBox);
            res_textures.erase(res_textures.begin() + selected.index);
            if (selected.index < uiTextures->getChildren().size())
            {
                ((UICheckBox*)uiTextures->getChildren()[selected.index])->setIsChecked(true);
            }
            else if (selected.index > 0)
            {
                ((UICheckBox*)uiTextures->getChildren()[selected.index - 1])->setIsChecked(true);
            }
        }
    };
}

void MainVC::hookCmd(sTextureCmd* cmd, UIControl* pCtrl)
{
    if (dynamic_cast<sTextureCmdFILL*>(cmd))
    {
        sTextureCmdFILL* pCmd = (sTextureCmdFILL*)cmd;

        auto colorPicker = pCtrl->getChild<UIPanel>("color");

        colorPicker->color = sUIColor{pCmd->color.x, pCmd->color.y, pCmd->color.z, pCmd->color.w};
        colorPicker->onClick = [this, pCmd, pCtrl, colorPicker](UIControl* pCtrl, const UIMouseEvent& evt)
        {
            showColorPicker(pCmd->color, [this, pCmd, pCtrl, colorPicker](const Color& color){
                pCmd->color = color;
                colorPicker->color = sUIColor{color.x, color.y, color.z, color.w};
                workingTexture->bake();
            });
        };
    }
    else if (dynamic_cast<sTextureCmdRECT*>(cmd))
    {
        sTextureCmdRECT* pCmd = (sTextureCmdRECT*)cmd;
    }
    else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
    {
        sTextureCmdBEVEL* pCmd = (sTextureCmdBEVEL*)cmd;
    }
    else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
    {
        sTextureCmdCIRCLE* pCmd = (sTextureCmdCIRCLE*)cmd;
    }
    else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
    {
        sTextureCmdBEVEL_CIRCLE* pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;
    }
    else if (dynamic_cast<sTextureCmdLINE*>(cmd))
    {
        sTextureCmdLINE* pCmd = (sTextureCmdLINE*)cmd;
    }
    else if (dynamic_cast<sTextureCmdNORMAL_MAP*>(cmd))
    {
        sTextureCmdNORMAL_MAP* pCmd = (sTextureCmdNORMAL_MAP*)cmd;
    }
    else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
    {
        sTextureCmdGRADIENT* pCmd = (sTextureCmdGRADIENT*)cmd;
    }
}

void MainVC::insertCmd(sTextureCmd* pCmd, UIControl* pCtrl)
{
    pCmd->texture = workingTexture;
    pCtrl->pUserData = pCmd;
    auto selected = getSelectedCmd();
    auto index = selected.index;
    if (index < workingTexture->cmds.size()) ++index;
    uiCmdStack->insertAfter(pCtrl, selected.selectBox);
    workingTexture->cmds.insert(workingTexture->cmds.begin() + index, pCmd);
    ((UICheckBox*)pCtrl)->setIsChecked(true);

    workingTexture->bake();

    hookCmd(pCmd, pCtrl);
}

void MainVC::addCmd(sTextureCmd* pCmd, UIControl* pCtrl)
{
    pCmd->texture = workingTexture;
    pCtrl->pUserData = pCmd;
    uiCmdStack->add(pCtrl);

    hookCmd(pCmd, pCtrl);
}

MainVC::sSelectTextureInfo MainVC::getSelectedTexture() const
{
    size_t i = 0;
    for (auto& pChild : uiTextures->getChildren())
    {
        if (((UICheckBox*)pChild)->getIsChecked()) return {i, (UICheckBox*)pChild, (sTexture*)pChild->pUserData};
        ++i;
    }
    return {0};
}

MainVC::sSelectCmdInfo MainVC::getSelectedCmd() const
{
    size_t i = 0;
    for (auto& pChild : uiCmdStack->getChildren())
    {
        if (((UICheckBox*)pChild)->getIsChecked()) return {i, (UICheckBox*)pChild, (sTextureCmd*)pChild->pUserData};
        ++i;
    }
    return {0};
}

void MainVC::closeAllViews()
{
    uiTextures->isVisible = false;
    uiTexture->isVisible = false;
    uiInspectorTextures->isVisible = false;
    uiInspectorTexture->isVisible = false;
}

#define TEXTURES_LAYOUT_PADDING 16
void MainVC::update()
{
    // Layout textures in the texture view
    if (uiTextures->isVisible)
    {
        sUIRect rect = {{TEXTURES_LAYOUT_PADDING, TEXTURES_LAYOUT_PADDING}, {128, 128}};
        auto psize = uiTextures->getWorldRect(uiContext).size;
        for (auto& pChild : uiTextures->getChildren())
        {
            pChild->rect = rect;
            rect.position.x += rect.size.x + TEXTURES_LAYOUT_PADDING;
            if (rect.position.x + rect.size.x + TEXTURES_LAYOUT_PADDING > psize.x)
            {
                rect.position.x = TEXTURES_LAYOUT_PADDING;
                rect.position.y += rect.size.y + TEXTURES_LAYOUT_PADDING;
            }
        }
    }

    // Layout commands
    if (uiTexture->isVisible)
    {
        sUIVector2 pos = {0, 0};
        for (auto& pChild : uiCmdStack->getChildren())
        {
            pChild->rect.position = pos;
            pos.y += pChild->rect.size.y;
        }

        // If user presses delete, delete selected command
        if (OInput->isStateJustDown(DIK_DELETE) &&
            !dynamic_cast<UITextBox*>(uiContext.getFocusControl()))
        {
            auto selected = getSelectedCmd();
            if (selected.cmd)
            {
                uiContext.clearState();
                uiCmdStack->remove(selected.selectBox);
                workingTexture->cmds.erase(workingTexture->cmds.begin() + selected.index);
                workingTexture->bake();
            }
        }
    }

    uiScreen.update(uiContext, {OMousePos.x, OMousePos.y}, OInput->isStateDown(DIK_MOUSEB1));
}

void MainVC::render()
{
    OSB->begin();
    uiScreen.render(uiContext);
    OSB->end();
}

void MainVC::buildUIForTexture()
{
    uiCmdStack->removeAll();
    for (auto cmd : workingTexture->cmds)
    {
        if (dynamic_cast<sTextureCmdFILL*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_FILL]->copy());
        }
        else if (dynamic_cast<sTextureCmdRECT*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_RECT]->copy());
        }
        else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_BEVEL]->copy());
        }
        else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_CIRCLE]->copy());
        }
        else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_BEVEL_CIRCLE]->copy());
        }
        else if (dynamic_cast<sTextureCmdLINE*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_LINE]->copy());
        }
        else if (dynamic_cast<sTextureCmdNORMAL_MAP*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_NORMAL_MAP]->copy());
        }
        else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
        {
            addCmd(cmd, cmdControls[eRES_CMD::RES_GRADIENT]->copy());
        }
    }
    uiTextureW->setInt(workingTexture->w);
    uiTextureH->setInt(workingTexture->h);
    pnlTexture->rect.size = {(float)workingTexture->w, (float)workingTexture->h};
    workingTexture->bake();
}

void MainVC::showColorPicker(const Color& color, function<void(const Color&)> callback)
{
    colorPickerCallback = callback;
    uiColorPicker->isVisible = true;

    colorPickerSliders[0] = (int)(color.x * 255.f);
    colorPickerSliders[1] = (int)(color.y * 255.f);
    colorPickerSliders[2] = (int)(color.z * 255.f);
    colorPickerSliders[3] = (int)(color.w * 255.f);

    uiColorPicker->getChild<UIPanel>("R")->pUserData = reinterpret_cast<void*>((uintptr_t)0);
    uiColorPicker->getChild<UIPanel>("G")->pUserData = reinterpret_cast<void*>((uintptr_t)1);
    uiColorPicker->getChild<UIPanel>("B")->pUserData = reinterpret_cast<void*>((uintptr_t)2);
    uiColorPicker->getChild<UIPanel>("A")->pUserData = reinterpret_cast<void*>((uintptr_t)3);

    uiColorPicker->getChild<UIPanel>("pnlColor")->color = sUIColor{color.x, color.y, color.z, color.w};
    uiColorPicker->getChild<UIPanel>("pnlPrevColor")->color = sUIColor{color.x, color.y, color.z, color.w};

    uiColorPicker->getChild<UIButton>("btnOK")->onClick = [this, &color](UIControl* c, const UIMouseEvent& e)
    {
        colorPickerCallback(Color{
            (float)colorPickerSliders[0] / 255.f,
            (float)colorPickerSliders[1] / 255.f,
            (float)colorPickerSliders[2] / 255.f,
            (float)colorPickerSliders[3] / 255.f});
        uiColorPicker->isVisible = false;
    };
    uiColorPicker->getChild<UIButton>("btnCancel")->onClick = [this, &color](UIControl* c, const UIMouseEvent& e)
    {
        uiColorPicker->isVisible = false;
    };
    auto mouseDownLambda = [this](UIControl* c, const UIMouseEvent& e)
    {
        if (c == uiContext.getDownControl())
        {
            auto colorAmount = 256 - (int)e.localMousePos.y;
            if (c == uiColorPicker->getChild<UIPanel>("R"))
            {
                colorPickerSliders[0] = max<>(0, min<>(255, colorAmount));
            }
            if (c == uiColorPicker->getChild<UIPanel>("G"))
            {
                colorPickerSliders[1] = max<>(0, min<>(255, colorAmount));
            }
            if (c == uiColorPicker->getChild<UIPanel>("B"))
            {
                colorPickerSliders[2] = max<>(0, min<>(255, colorAmount));
            }
            if (c == uiColorPicker->getChild<UIPanel>("A"))
            {
                colorPickerSliders[3] = max<>(0, min<>(255, colorAmount));
            }
            updateColorPickerValues();
        }
    };

    uiColorPicker->getChild<UIPanel>("R")->onMouseMove = mouseDownLambda;
    uiColorPicker->getChild<UIPanel>("G")->onMouseMove = mouseDownLambda;
    uiColorPicker->getChild<UIPanel>("B")->onMouseMove = mouseDownLambda;
    uiColorPicker->getChild<UIPanel>("A")->onMouseMove = mouseDownLambda;

    uiColorPicker->getChild<UITextBox>("txtR")->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        colorPickerSliders[0] = c->getInt();
        updateColorPickerValues();
    };
    uiColorPicker->getChild<UITextBox>("txtG")->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        colorPickerSliders[1] = c->getInt();
        updateColorPickerValues();
    };
    uiColorPicker->getChild<UITextBox>("txtB")->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        colorPickerSliders[2] = c->getInt();
        updateColorPickerValues();
    };
    uiColorPicker->getChild<UITextBox>("txtA")->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        colorPickerSliders[3] = c->getInt();
        updateColorPickerValues();
    };
}

void MainVC::updateColorPickerValues()
{
    uiColorPicker->getChild<UIPanel>("pnlColor")->color = sUIColor{
        (float)colorPickerSliders[0] / 255.f,
        (float)colorPickerSliders[1] / 255.f,
        (float)colorPickerSliders[2] / 255.f,
        1};
    uiColorPicker->getChild<UITextBox>("txtR")->setInt(colorPickerSliders[0]);
    uiColorPicker->getChild<UITextBox>("txtG")->setInt(colorPickerSliders[1]);
    uiColorPicker->getChild<UITextBox>("txtB")->setInt(colorPickerSliders[2]);
    uiColorPicker->getChild<UITextBox>("txtA")->setInt(colorPickerSliders[3]);
}
