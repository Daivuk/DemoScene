#include "MainVC.h"
#include "styles.h"
#include "compress.h"

using namespace std;

static const Color g_guideColor = OColorHex(3399ff);
static const Color g_toolBtnHoverColor = OColorHex(3e3e40);
static const Color g_toolBtnDownColor = OColorHex(007acc);
static const Color g_toolColorSliderBG = OColorHex(000000);

int colorPickerSliders[4];

extern vector<res_Color*>* allColors;
void updatePalette()
{
    res_palette.clear();
    if (allColors)
    {
        for (auto c : *allColors)
        {
            res_getColorId(*c);
        }
    }
}

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
        orect = orect.Grow(-2);
        if (resT->texture[CHANNEL_DIFFUSE])
        {
            float ratio = resT->texture[CHANNEL_DIFFUSE]->getSizef().x / resT->texture[CHANNEL_DIFFUSE]->getSizef().y;
            if (ratio > 1)
            {
                orect.w /= ratio;
            }
            else
            {
                orect.z *= ratio;
            }
            OSB->drawRectWithUVs(OGetTexture("dottedLine.png"), orect, {0, 0, orect.z / 16, orect.w / 16}, g_toolBtnHoverColor);
            OSB->drawRect(resT->texture[CHANNEL_DIFFUSE], orect);
        }
    });

    uiPnlTexture = uiScreen.getChild("pnlTexture");
    uiContext.addStyle<UIPanel>("pnlTexture", [this](const onut::UIPanel* pCheckBox, const onut::sUIRect& rect)
    {
        auto state = pCheckBox->getState(uiContext);
        auto orect = UI2Onut(rect);
        if (OInput->isStateDown(DIK_SPACE))
        {
            // Render with shaders and shit
            if (workingTexture->texture[CHANNEL_DIFFUSE])
            {
                OSB->drawRect(workingTexture->texture[CHANNEL_DIFFUSE], orect);
            }
        }
        else
        {
            OSB->drawRectWithUVs(OGetTexture("dottedLine.png"), orect, {0, 0, orect.z / 16, orect.w / 16}, g_toolBtnHoverColor);
            if (workingTexture->texture[workingChannel])
            {
                OSB->drawRect(workingTexture->texture[workingChannel], orect);
            }
        }
    });

    uiContext.addStyle<UIPanel>("buttonSelection", [this](const onut::UIPanel* pCheckBox, const onut::sUIRect& rect)
    {
        auto orect = UI2Onut(rect);
        OSB->drawRect(nullptr, orect.Grow(2), g_guideColor);
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
    uiColorPickerPreviousContainer = uiColorPicker->getChild("previous");
    uiColorPickerPreviousSample = uiColorPicker->getChild("pnlPrevious");
    uiColorPickerPreviousSample->retain();
    uiColorPickerPreviousSample->remove();
    uiCmdStack = uiInspectorTexture->getChild("pnlCmdStack");
    uiSaved = uiScreen.getChild("lblSaved");
    uiSavedY = uiSaved->rect.position.y;
    uiSavedV = uiSaved->isVisible ? 1 : 0;
    cmdControls[eRES_CMD::RES_FILL] = uiInspectorTexture->getChild<UICheckBox>("chkCmdFILL");
    cmdControls[eRES_CMD::RES_RECT] = uiInspectorTexture->getChild<UICheckBox>("chkCmdRECT");
    cmdControls[eRES_CMD::RES_BEVEL] = uiInspectorTexture->getChild<UICheckBox>("chkCmdBEVEL");
    cmdControls[eRES_CMD::RES_CIRCLE] = uiInspectorTexture->getChild<UICheckBox>("chkCmdCIRCLE");
    cmdControls[eRES_CMD::RES_BEVEL_CIRCLE] = uiInspectorTexture->getChild<UICheckBox>("chkCmdBEVEL_CIRCLE");
    cmdControls[eRES_CMD::RES_LINE] = uiInspectorTexture->getChild<UICheckBox>("chkCmdLINE");
    cmdControls[eRES_CMD::RES_NORMAL_MAP] = uiInspectorTexture->getChild<UICheckBox>("chkCmdNORMAL_MAP");
    cmdControls[eRES_CMD::RES_GRADIENT] = uiInspectorTexture->getChild<UICheckBox>("chkCmdGRADIENT");
    cmdControls[eRES_CMD::RES_IMAGE] = uiInspectorTexture->getChild<UICheckBox>("chkCmdIMAGE");
    uiInspectorTexture->getChild<UIButton>("btnDiffuse")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        workingChannel = CHANNEL_DIFFUSE;
        buildUIForTexture();
        uiInspectorTexture->getChild<UIPanel>("channelSelection")->rect = c->rect;
    };
    uiInspectorTexture->getChild<UIButton>("btnNormal")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        workingChannel = CHANNEL_NORMAL;
        buildUIForTexture();
        uiInspectorTexture->getChild<UIPanel>("channelSelection")->rect = c->rect;
    };
    uiInspectorTexture->getChild<UIButton>("btnMaterial")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        workingChannel = CHANNEL_MATERIAL;
        buildUIForTexture();
        uiInspectorTexture->getChild<UIPanel>("channelSelection")->rect = c->rect;
    };
    uiInspectorTexture->getChild<UIButton>("btnDelChannel")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        workingTexture->cmds[workingChannel].clear();
        workingTexture->bake(workingChannel);
        buildUIForTexture();
    };
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
        workingTexture->bake(CHANNEL_DIFFUSE);
        workingTexture->bake(CHANNEL_NORMAL);
        workingTexture->bake(CHANNEL_MATERIAL);
    };
    uiTextureH->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        auto d = uiTextureH->getInt();
        pnlTexture->rect.size.y = (float)d;
        workingTexture->h = d;
        workingTexture->bake(CHANNEL_DIFFUSE);
        workingTexture->bake(CHANNEL_NORMAL);
        workingTexture->bake(CHANNEL_MATERIAL);
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
    //uiInspectorTexture->getChild("btnCmdNORMAL_MAP")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    //{
    //    insertCmd(new sTextureCmdNORMAL_MAP(), cmdControls[eRES_CMD::RES_NORMAL_MAP]->copy());
    //};
    //uiInspectorTexture->getChild("btnCmdGRADIENT")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    //{
    //    insertCmd(new sTextureCmdGRADIENT(), cmdControls[eRES_CMD::RES_GRADIENT]->copy());
    //};
    uiInspectorTexture->getChild("btnCmdIMAGE")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        insertCmd(new sTextureCmdIMAGE(), cmdControls[eRES_CMD::RES_IMAGE]->copy());
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
    auto editLambda = [this](UIControl* c, const UIMouseEvent& e)
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
    uiInspectorTextures->getChild("btnNewTexture")->onClick = [this, editLambda](UIControl* c, const UIMouseEvent& e)
    {
        auto pTexture = new sTexture();
        auto pSelectBox = new UICheckBox();

        pSelectBox->rect.size = {128, 128};
        pSelectBox->pUserData = pTexture;
        pSelectBox->setStyle("selectTexture");
        pSelectBox->behavior = eUICheckBehavior::EXCLUSIVE;
        pSelectBox->onDoubleClick = editLambda;

        auto selected = getSelectedTexture();
        auto index = selected.index;
        if (index < res_textures.size()) ++index;
        uiTextures->insertAfter(pSelectBox, selected.selectBox);
        res_textures.insert(res_textures.begin() + index, pTexture);

        pSelectBox->setIsChecked(true);

        shiftTextureReferences(index, 1);
    };
    uiInspectorTextures->getChild("btnEditTexture")->onClick = editLambda;
    uiInspectorTextures->getChild("btnDeleteTexture")->onClick = [this](UIControl* c, const UIMouseEvent& e)
    {
        auto selected = getSelectedTexture();
        if (selected.texture)
        {
            uiContext.clearState();
            uiTextures->remove(selected.selectBox);
            res_textures.erase(res_textures.begin() + selected.index);
            shiftTextureReferences(selected.index, -1);
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
    uiTexture->onMouseDown = [this](UIControl* c, const UIMouseEvent& e)
    {
        onDownMousePos = {e.mousePos.x, e.mousePos.y};
        auto offset = uiPnlTexture->getWorldRect(uiContext).position;
        onDownMousePos.x -= offset.x;
        onDownMousePos.y -= offset.y;
        lastDragDiff = {0, 0};
        dragId = pickOnTexture(onDownMousePos, downState);
    };
    uiTexture->onMouseMove = [this](UIControl* c, const UIMouseEvent& e)
    {
        if (c != uiContext.getDownControl()) return;
        if (dragId == -1) return;
        auto offset = uiPnlTexture->getWorldRect(uiContext).position;
        auto mousePos = e.mousePos;
        mousePos.x -= offset.x;
        mousePos.y -= offset.y;
        Vector2 mouseDiff{mousePos.x - onDownMousePos.x, mousePos.y - onDownMousePos.y};
        if (mouseDiff != lastDragDiff)
        {
            updateTextureEdit(mouseDiff, Vector2{mousePos.x, mousePos.y});
            lastDragDiff = mouseDiff;
        }
    };
    uiTexture->onMouseUp = [this](UIControl* c, const UIMouseEvent& e)
    {
        dragId = -1;
    };

    load();
}

extern MainVC* pMainVC;

void hookColorPicker(UIControl* pCtrl, const std::string& childName, res_Color* pTarget)
{
    auto colorPicker = pCtrl->getChild<UIPanel>(childName);
    colorPicker->color = sUIColor{
        (float)pTarget->x / 255.f, 
        (float)pTarget->y / 255.f,
        (float)pTarget->z / 255.f,
        (float)pTarget->w / 255.f};
    colorPicker->onClick = [pCtrl, colorPicker, pTarget](UIControl* pCtrl, const UIMouseEvent& evt)
    {
        pMainVC->showColorPicker(*pTarget, [pCtrl, colorPicker, pTarget](const res_palColor& color){
            *pTarget = color;
            colorPicker->color = sUIColor{
                (float)color.x / 255.f,
                (float)color.y / 255.f,
                (float)color.z / 255.f,
                (float)color.w / 255.f};
            pMainVC->workingTexture->bake(pMainVC->workingChannel);
        });
    };
}

void hookInteger(UIControl* pCtrl, const std::string& childName, int* pTarget, int _min, int _max)
{
    auto size = pCtrl->getChild<UITextBox>(childName);
    size->setInt(*pTarget);
    size->onTextChanged = [=](UITextBox* pCtrl, const UITextBoxEvent& evt)
    {
        int val = pCtrl->getInt();
        int correctedVal = clamp(val, _min, _max);
        if (val != correctedVal)
        {
            pCtrl->setInt(correctedVal);
        }
        *pTarget = val;
        pMainVC->workingTexture->bake(pMainVC->workingChannel);
    };
}

void MainVC::hookCmd(sTextureCmd* cmd, UIControl* pCtrl)
{
    if (dynamic_cast<sTextureCmdFILL*>(cmd))
    {
        auto pCmd = (sTextureCmdFILL*)cmd;
        hookColorPicker(pCtrl, "color", &pCmd->color);
    }
    else if (dynamic_cast<sTextureCmdRECT*>(cmd))
    {
        auto pCmd = (sTextureCmdRECT*)cmd;
        hookColorPicker(pCtrl, "color", &pCmd->color);
    }
    else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
    {
        auto pCmd = (sTextureCmdBEVEL*)cmd;
        hookColorPicker(pCtrl, "color", &pCmd->color);
        hookInteger(pCtrl, "txtBevel", &pCmd->bevel, 1, 64);
    }
    else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
    {
        auto pCmd = (sTextureCmdCIRCLE*)cmd;
        hookColorPicker(pCtrl, "color", &pCmd->color);
    }
    else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
    {
        auto pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;
        hookColorPicker(pCtrl, "color", &pCmd->color);
        hookInteger(pCtrl, "txtBevel", &pCmd->bevel, 1, 64);
    }
    else if (dynamic_cast<sTextureCmdLINE*>(cmd))
    {
        auto pCmd = (sTextureCmdLINE*)cmd;
        hookColorPicker(pCtrl, "color", &pCmd->color);
        hookInteger(pCtrl, "txtSize", &pCmd->size, 1, 64);
    }
    else if (dynamic_cast<sTextureCmdNORMAL_MAP*>(cmd))
    {
        auto pCmd = (sTextureCmdNORMAL_MAP*)cmd;
    }
    else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
    {
        auto pCmd = (sTextureCmdGRADIENT*)cmd;
        hookColorPicker(pCtrl, "colorFrom", &pCmd->color1);
        hookColorPicker(pCtrl, "colorTo", &pCmd->color1);
    }
    else if (dynamic_cast<sTextureCmdIMAGE*>(cmd))
    {
        auto pCmd = (sTextureCmdIMAGE*)cmd;
        hookColorPicker(pCtrl, "color", &pCmd->color);
        hookInteger(pCtrl, "txtBevel", &pCmd->imgId, 0, 255);
    }
}

void MainVC::insertCmd(sTextureCmd* pCmd, UIControl* pCtrl)
{
    pCmd->texture = workingTexture;
    pCtrl->pUserData = pCmd;
    auto selected = getSelectedCmd();
    auto index = selected.index;
    if (index < workingTexture->cmds[workingChannel].size()) ++index;
    uiCmdStack->insertAt(pCtrl, index);
    workingTexture->cmds[workingChannel].insert(workingTexture->cmds[workingChannel].begin() + index, pCmd);
    ((UICheckBox*)pCtrl)->setIsChecked(true);

    workingTexture->bake(workingChannel);

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

    if (uiTextures->isVisible)
    {
        auto selected = getSelectedTexture();
        if (selected.texture)
        {
            if (OInput->isStateJustDown(DIK_D))
            {
                auto pTexture = selected.texture->copy();
                auto pSelectBox = new UICheckBox();

                pSelectBox->rect.size = {128, 128};
                pSelectBox->pUserData = pTexture;
                pSelectBox->setStyle("selectTexture");
                pSelectBox->behavior = eUICheckBehavior::EXCLUSIVE;
                pSelectBox->onDoubleClick = [this](UIControl* c, const UIMouseEvent& e)
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

                auto index = selected.index;
                if (index < res_textures.size()) ++index;
                uiTextures->insertAfter(pSelectBox, selected.selectBox);
                res_textures.insert(res_textures.begin() + index, pTexture);

                pSelectBox->setIsChecked(true);

                // Shift other references
                shiftTextureReferences((int)index, 1);
            }
        }
    }
    if (uiTexture->isVisible)
    {
        // Layout commands
        sUIVector2 pos = {0, cmdStackOffset.get()};
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
                workingTexture->cmds[workingChannel].erase(workingTexture->cmds[workingChannel].begin() + selected.index);
                workingTexture->bake(workingChannel);
            }
        }

        // Scroll
        if (OInput->getStateValue(DIK_MOUSEZ) > 0)
        {
            auto curVal = cmdStackOffset.get();
            cmdStackOffset.stop(true);
            auto endVal = cmdStackOffset.get();
            endVal += 50.f;
            if (endVal > 0)
            {
                endVal = 0;
            }
            cmdStackOffset.start(curVal, endVal, .15f, OEaseOut);
        }
        else if (OInput->getStateValue(DIK_MOUSEZ) < 0)
        {
            auto curVal = cmdStackOffset.get();
            cmdStackOffset.stop(true);
            auto endVal = cmdStackOffset.get();
            endVal -= 50.f;
            auto stackH = uiCmdStack->getWorldRect(uiContext).size.y;
            if (endVal < (float)workingTexture->cmds[workingChannel].size() * -50.f + stackH)
            {
                endVal = (float)workingTexture->cmds[workingChannel].size() * -50.f + stackH;
            }
            cmdStackOffset.start(curVal, endVal, .15f, OEaseOut);
        }

        // Actions on the selected cmd
        auto selectedCmd = getSelectedCmd();
        if (selectedCmd.cmd)
        {
            if (OInput->isStateJustDown(DIK_D))
            {
                insertCmd(selectedCmd.cmd->copy(), cmdControls[selectedCmd.cmd->getType()]->copy());
            }
            else if (OInput->isStateJustDown(DIK_DOWN) &&
                     OInput->isStateDown(DIK_LCONTROL))
            {
                if (selectedCmd.index < workingTexture->cmds[workingChannel].size() - 1)
                {
                    selectedCmd.selectBox->retain();
                    selectedCmd.selectBox->remove();
                    workingTexture->cmds[workingChannel].erase(workingTexture->cmds[workingChannel].begin() + selectedCmd.index);
                    ++selectedCmd.index;

                    uiCmdStack->insertAt(selectedCmd.selectBox, selectedCmd.index);
                    workingTexture->cmds[workingChannel].insert(workingTexture->cmds[workingChannel].begin() + selectedCmd.index, selectedCmd.cmd);

                    selectedCmd.selectBox->release();

                    workingTexture->bake(workingChannel);
                }
            }
            else if (OInput->isStateJustDown(DIK_UP) &&
                     OInput->isStateDown(DIK_LCONTROL))
            {
                if (selectedCmd.index > 0)
                {
                    selectedCmd.selectBox->retain();
                    selectedCmd.selectBox->remove();
                    workingTexture->cmds[workingChannel].erase(workingTexture->cmds[workingChannel].begin() + selectedCmd.index);
                    --selectedCmd.index;

                    uiCmdStack->insertAt(selectedCmd.selectBox, selectedCmd.index);
                    workingTexture->cmds[workingChannel].insert(workingTexture->cmds[workingChannel].begin() + selectedCmd.index, selectedCmd.cmd);

                    selectedCmd.selectBox->release();

                    workingTexture->bake(workingChannel);
                }
            }
        }
    }
    if (OInput->isStateJustDown(DIK_G))
    {
        bShowGrid = !bShowGrid;
    }
    if (OInput->isStateJustDown(DIK_S) && OInput->isStateDown(DIK_LCONTROL))
    {
        save();
    }

    // Update the little save popup position
    uiSaved->rect.position.y = uiSavedY.get();
    uiSaved->isVisible = uiSavedV.get() ? true : false;

    uiScreen.update(uiContext, {OMousePos.x, OMousePos.y}, OInput->isStateDown(DIK_MOUSEB1));
}

void MainVC::shiftTextureReferences(int index, int inc)
{
    for (auto texture : res_textures)
    {
        for (auto channel = 0; channel < 3; ++channel)
        {
            for (auto cmd : texture->cmds[channel])
            {
                if (cmd->getType() == RES_IMAGE)
                {
                    auto& id = ((sTextureCmdIMAGE*)cmd)->imgId;
                    if (id >= (int)index)
                    {
                        id += inc;
                    }
                }
            }
        }
    }
}

void MainVC::render()
{
    OSB->begin();
    uiScreen.render(uiContext);
    OSB->end();

    // Now draw selected shape
    if (uiTexture->isVisible && !uiColorPicker->isVisible)
    {
        auto rect = uiPnlTexture->getWorldRect(uiContext);
        if (bShowGrid)
        {
            const Color gridColor{0, .75f, 1, .25f};
            OPB->begin(ePrimitiveType::LINES);
            for (float x = rect.position.x + 16; x < (rect.position.x + rect.size.x); x += 16)
            {
                OPB->draw({x, rect.position.y}, gridColor);
                OPB->draw({x, rect.position.y + rect.size.y}, gridColor);
            }
            for (float x = rect.position.y + 16; x < (rect.position.y + rect.size.y); x += 16)
            {
                OPB->draw({rect.position.x, x}, gridColor);
                OPB->draw({rect.position.x + rect.size.x, x}, gridColor);
            }
            OPB->end();
        }

        auto cmd = getSelectedCmd().cmd;
        const Color Magenta(1, 0, 1, .5f);
        if (cmd)
        {
            auto offset = rect.position;
            if (dynamic_cast<sTextureCmdFILL*>(cmd))
            {
                sTextureCmdFILL* pCmd = (sTextureCmdFILL*)cmd;
            }
            else if (dynamic_cast<sTextureCmdRECT*>(cmd))
            {
                sTextureCmdRECT* pCmd = (sTextureCmdRECT*)cmd;
                OPB->begin(ePrimitiveType::LINE_STRIP);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->end();
            }
            else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
            {
                sTextureCmdBEVEL* pCmd = (sTextureCmdBEVEL*)cmd;
                OPB->begin(ePrimitiveType::LINE_STRIP);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->end();
            }
            else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
            {
                sTextureCmdCIRCLE* pCmd = (sTextureCmdCIRCLE*)cmd;
                OPB->begin(ePrimitiveType::LINES);
                OPB->draw({offset.x + (float)pCmd->x - 4, offset.y + (float)pCmd->y - 4}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x + 4, offset.y + (float)pCmd->y + 4}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x + 4, offset.y + (float)pCmd->y - 4}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x - 4, offset.y + (float)pCmd->y + 4}, Magenta);
                OPB->end();
                OPB->begin(ePrimitiveType::LINE_STRIP);
                for (int angle = 0; angle <= 360; angle += 10)
                {
                    OPB->draw({
                        offset.x + (float)pCmd->x + cosf(DirectX::XMConvertToRadians((float)angle)) * (float)pCmd->radius,
                        offset.y + (float)pCmd->y + sinf(DirectX::XMConvertToRadians((float)angle)) * (float)pCmd->radius
                    }, Magenta);
                }
                OPB->end();
            }
            else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
            {
                sTextureCmdBEVEL_CIRCLE* pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;
                OPB->begin(ePrimitiveType::LINES);
                OPB->draw({offset.x + (float)pCmd->x - 4, offset.y + (float)pCmd->y - 4}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x + 4, offset.y + (float)pCmd->y + 4}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x + 4, offset.y + (float)pCmd->y - 4}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x - 4, offset.y + (float)pCmd->y + 4}, Magenta);
                OPB->end();
                OPB->begin(ePrimitiveType::LINE_STRIP);
                for (int angle = 0; angle <= 360; angle += 10)
                {
                    OPB->draw({
                        offset.x + (float)pCmd->x + cosf(DirectX::XMConvertToRadians((float)angle)) * (float)pCmd->radius,
                        offset.y + (float)pCmd->y + sinf(DirectX::XMConvertToRadians((float)angle)) * (float)pCmd->radius
                    }, Magenta);
                }
                OPB->end();
                OPB->begin(ePrimitiveType::LINE_STRIP);
                for (int angle = 0; angle <= 360; angle += 10)
                {
                    OPB->draw({
                        offset.x + (float)pCmd->x + cosf(DirectX::XMConvertToRadians((float)angle)) * (float)(pCmd->radius - pCmd->bevel),
                        offset.y + (float)pCmd->y + sinf(DirectX::XMConvertToRadians((float)angle)) * (float)(pCmd->radius - pCmd->bevel)
                    }, Magenta);
                }
                OPB->end();
            }
            else if (dynamic_cast<sTextureCmdLINE*>(cmd))
            {
                sTextureCmdLINE* pCmd = (sTextureCmdLINE*)cmd;
                OPB->begin(ePrimitiveType::LINES);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y2}, Magenta);
                OPB->end();
            }
            else if (dynamic_cast<sTextureCmdNORMAL_MAP*>(cmd))
            {
                sTextureCmdNORMAL_MAP* pCmd = (sTextureCmdNORMAL_MAP*)cmd;
            }
            else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
            {
                sTextureCmdGRADIENT* pCmd = (sTextureCmdGRADIENT*)cmd;
                OPB->begin(ePrimitiveType::LINE_STRIP);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->end();
            }
            else if (dynamic_cast<sTextureCmdIMAGE*>(cmd))
            {
                auto* pCmd = (sTextureCmdIMAGE*)cmd;
                OPB->begin(ePrimitiveType::LINE_STRIP);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y2}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x2, offset.y + (float)pCmd->y1}, Magenta);
                OPB->draw({offset.x + (float)pCmd->x1, offset.y + (float)pCmd->y1}, Magenta);
                OPB->end();
            }
        }
    }
}

void MainVC::buildUIForTexture()
{
    uiCmdStack->removeAll();
    for (auto cmd : workingTexture->cmds[workingChannel])
    {
        addCmd(cmd, cmdControls[cmd->getType()]->copy());
    }
    uiTextureW->setInt(workingTexture->w);
    uiTextureH->setInt(workingTexture->h);
    pnlTexture->rect.size = {(float)workingTexture->w, (float)workingTexture->h};
    workingTexture->bake(workingChannel);
}

void MainVC::showColorPicker(const res_palColor& color, function<void(const res_palColor&)> callback)
{
    colorPickerCallback = callback;
    uiColorPicker->isVisible = true;

    colorPickerSliders[0] = (int)color.x;
    colorPickerSliders[1] = (int)color.y;
    colorPickerSliders[2] = (int)color.z;
    colorPickerSliders[3] = (int)color.w;

    uiColorPicker->getChild<UIPanel>("R")->pUserData = reinterpret_cast<void*>((uintptr_t)0);
    uiColorPicker->getChild<UIPanel>("G")->pUserData = reinterpret_cast<void*>((uintptr_t)1);
    uiColorPicker->getChild<UIPanel>("B")->pUserData = reinterpret_cast<void*>((uintptr_t)2);
    uiColorPicker->getChild<UIPanel>("A")->pUserData = reinterpret_cast<void*>((uintptr_t)3);

    uiColorPicker->getChild<UIPanel>("pnlColor")->color = {
        (float)color.x / 255.f,
        (float)color.y / 255.f,
        (float)color.z / 255.f,
        (float)color.w / 255.f};
    auto prevColor = uiColorPicker->getChild<UIPanel>("pnlPrevColor");
    prevColor->color = {
        (float)color.x / 255.f,
        (float)color.y / 255.f,
        (float)color.z / 255.f,
        (float)color.w / 255.f};

    prevColor->onClick = [&](UIControl* pCtrl, const UIMouseEvent& e)
    {
        colorPickerSliders[0] = color.x;
        colorPickerSliders[1] = color.y;
        colorPickerSliders[2] = color.z;
        colorPickerSliders[3] = color.w;
        updateColorPickerValues();
    };

    uiColorPicker->getChild<UIButton>("btnOK")->onClick = [this, &color](UIControl* c, const UIMouseEvent& e)
    {
        colorPickerCallback(res_palColor{
            (uint8_t)colorPickerSliders[0],
            (uint8_t)colorPickerSliders[1],
            (uint8_t)colorPickerSliders[2],
            (uint8_t)colorPickerSliders[3]});
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
    uiColorPicker->getChild<UITextBox>("txtR")->setInt(colorPickerSliders[0]);
    uiColorPicker->getChild<UITextBox>("txtG")->setInt(colorPickerSliders[1]);
    uiColorPicker->getChild<UITextBox>("txtB")->setInt(colorPickerSliders[2]);
    uiColorPicker->getChild<UITextBox>("txtA")->setInt(colorPickerSliders[3]);

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

    // Setup previous
    uiColorPickerPreviousContainer->removeAll();
    updatePalette();
    auto rect = uiColorPickerPreviousSample->rect;
    auto contRect = uiColorPickerPreviousContainer->getWorldRect(uiContext);
    for (auto& c : res_palette)
    {
        auto sample = uiColorPickerPreviousSample->copy();
        sample->pUserData = &c;
        uiColorPickerPreviousContainer->add(sample);
        sample->rect = rect;
        ((UIPanel*)sample)->color = sUIColor{
            (float)c.x / 255.f,
            (float)c.y / 255.f,
            (float)c.z / 255.f,
            (float)c.w / 255.f};
        rect.position.x += rect.size.x;
        if (rect.position.x + rect.size.x > contRect.size.x)
        {
            rect.position.x = 0;
            rect.position.y += rect.size.y;
        }
        sample->onClick = [&](UIControl* pCtrl, const UIMouseEvent& e)
        {
            res_palColor* pColor = (res_palColor*)pCtrl->pUserData;
            colorPickerSliders[0] = pColor->x;
            colorPickerSliders[1] = pColor->y;
            colorPickerSliders[2] = pColor->z;
            colorPickerSliders[3] = pColor->w;
            updateColorPickerValues();
        };
    }
}

void MainVC::updateColorPickerValues()
{
    uiColorPicker->getChild<UIPanel>("pnlColor")->color = sUIColor{
        (float)colorPickerSliders[0] / 255.f,
        (float)colorPickerSliders[1] / 255.f,
        (float)colorPickerSliders[2] / 255.f,
        (float)colorPickerSliders[3] / 255.f};
    uiColorPicker->getChild<UITextBox>("txtR")->setInt(colorPickerSliders[0]);
    uiColorPicker->getChild<UITextBox>("txtG")->setInt(colorPickerSliders[1]);
    uiColorPicker->getChild<UITextBox>("txtB")->setInt(colorPickerSliders[2]);
    uiColorPicker->getChild<UITextBox>("txtA")->setInt(colorPickerSliders[3]);
}

template<typename Tcmd>
int pickOnCmd_RECT(Tcmd* pCmd, const Vector2& pos, int* out_downState)
{
    Vector2 p1{(float)pCmd->x1, (float)pCmd->y1};
    Vector2 p2{(float)pCmd->x2, (float)pCmd->y2};
    out_downState[0] = pCmd->x1;
    out_downState[1] = pCmd->y1;
    out_downState[2] = pCmd->x2;
    out_downState[3] = pCmd->y2;
    if (pos.x >= p1.x && pos.x <= p2.x && pos.y >= p1.y && pos.y <= p2.y) return 0;
    else if (pos.x < p1.x && pos.y < p1.y) return 1;
    else if (pos.x > p2.x && pos.y < p1.y) return 2;
    else if (pos.x > p2.x && pos.y > p2.y) return 3;
    else if (pos.x < p1.x && pos.y > p2.y) return 4;
    else if (pos.x < p1.x) return 5;
    else if (pos.y < p1.y) return 6;
    else if (pos.x > p2.x) return 7;
    else if (pos.y > p2.y) return 8;
    return -1;
}

template<typename Tcmd>
int pickOnCmd_CIRCLE(Tcmd* pCmd, const Vector2& pos, int* out_downState)
{
    Vector2 p{(float)pCmd->x, (float)pCmd->y};
    float dist = Vector2::DistanceSquared(p, pos);
    float radius = (float)max<>(8, pCmd->radius);
    if (dist <= radius * radius)
    {
        out_downState[0] = pCmd->x;
        out_downState[1] = pCmd->y;
        return 0;
    }
    else
    {
        out_downState[0] = pCmd->radius;
        out_downState[1] = (int)sqrtf(dist);
        return 1;
    }
}

int MainVC::pickOnTexture(const Vector2& pos, int* out_downState)
{
    out_downState[0] = 0;
    out_downState[1] = 0;
    out_downState[2] = 0;
    out_downState[3] = 0;
    auto cmd = getSelectedCmd().cmd;
    if (!cmd) return -1;

    if (dynamic_cast<sTextureCmdRECT*>(cmd))
    {
        return pickOnCmd_RECT((sTextureCmdRECT*)cmd, pos, out_downState);
    }
    else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
    {
        return pickOnCmd_RECT((sTextureCmdBEVEL*)cmd, pos, out_downState);
    }
    else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
    {
        return pickOnCmd_CIRCLE((sTextureCmdCIRCLE*)cmd, pos, out_downState);
    }
    else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
    {
        return pickOnCmd_CIRCLE((sTextureCmdBEVEL_CIRCLE*)cmd, pos, out_downState);
    }
    else if (dynamic_cast<sTextureCmdLINE*>(cmd))
    {
        sTextureCmdLINE* pCmd = (sTextureCmdLINE*)cmd;
        Vector2 p1{(float)pCmd->x1, (float)pCmd->y1};
        Vector2 p2{(float)pCmd->x2, (float)pCmd->y2};
        float dist1 = Vector2::DistanceSquared(p1, pos);
        float dist2 = Vector2::DistanceSquared(p2, pos);
        float radius = (float)max<>(8, pCmd->size);
        if (dist2 < dist1)
        {
            if (dist2 < radius * radius)
            {
                out_downState[0] = pCmd->x2;
                out_downState[1] = pCmd->y2;
                return 1;
            }
        }
        else
        {
            if (dist1 < radius * radius)
            {
                out_downState[0] = pCmd->x1;
                out_downState[1] = pCmd->y1;
                return 0;
            }
        }
    }
    else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
    {
        return pickOnCmd_RECT((sTextureCmdGRADIENT*)cmd, pos, out_downState);
    }
    else if (dynamic_cast<sTextureCmdIMAGE*>(cmd))
    {
        return pickOnCmd_RECT((sTextureCmdIMAGE*)cmd, pos, out_downState);
    }

    return -1;
}

int toCmdPos(int pos)
{
    pos = max<>(-32, pos);
    pos = min<>(988, pos);
    return pos / 4 * 4;
}

template<typename Tcmd>
void updateTextEdit_RECT(Tcmd* pCmd, const Vector2& diff, const Vector2& mousePos, int dragId, int* downState)
{
    if (dragId == 0)
    {
        pCmd->x1 = toCmdPos(downState[0] + (int)diff.x);
        pCmd->y1 = toCmdPos(downState[1] + (int)diff.y);
        pCmd->x2 = toCmdPos(downState[2] + (int)diff.x);
        pCmd->y2 = toCmdPos(downState[3] + (int)diff.y);
    }
    else if (dragId == 1)
    {
        pCmd->x1 = toCmdPos(downState[0] + (int)diff.x);
        pCmd->y1 = toCmdPos(downState[1] + (int)diff.y);
    }
    else if (dragId == 2)
    {
        pCmd->x2 = toCmdPos(downState[2] + (int)diff.x);
        pCmd->y1 = toCmdPos(downState[1] + (int)diff.y);
    }
    else if (dragId == 3)
    {
        pCmd->x2 = toCmdPos(downState[2] + (int)diff.x);
        pCmd->y2 = toCmdPos(downState[3] + (int)diff.y);
    }
    else if (dragId == 4)
    {
        pCmd->x1 = toCmdPos(downState[0] + (int)diff.x);
        pCmd->y2 = toCmdPos(downState[3] + (int)diff.y);
    }
    else if (dragId == 5) pCmd->x1 = toCmdPos(downState[0] + (int)diff.x);
    else if (dragId == 6) pCmd->y1 = toCmdPos(downState[1] + (int)diff.y);
    else if (dragId == 7) pCmd->x2 = toCmdPos(downState[2] + (int)diff.x);
    else if (dragId == 8) pCmd->y2 = toCmdPos(downState[3] + (int)diff.y);
}

void MainVC::updateTextureEdit(const Vector2& diff, const Vector2& mousePos)
{
    auto cmd = getSelectedCmd().cmd;
    if (!cmd) return;
    if (dragId == -1) return;

    if (dynamic_cast<sTextureCmdRECT*>(cmd))
    {
        updateTextEdit_RECT((sTextureCmdRECT*)cmd, diff, mousePos, dragId, downState);
    }
    else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
    {
        updateTextEdit_RECT((sTextureCmdBEVEL*)cmd, diff, mousePos, dragId, downState);
    }
    else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
    {
        sTextureCmdCIRCLE* pCmd = (sTextureCmdCIRCLE*)cmd;
        if (dragId == 0)
        {
            pCmd->x = toCmdPos(downState[0] + (int)diff.x);
            pCmd->y = toCmdPos(downState[1] + (int)diff.y);
        }
        else if (dragId == 1)
        {
            Vector2 p{(float)pCmd->x, (float)pCmd->y};
            float dist = Vector2::Distance(p, mousePos);
            pCmd->radius = downState[0] + ((int)dist - downState[1]);
        }
    }
    else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
    {
        sTextureCmdBEVEL_CIRCLE* pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;
        if (dragId == 0)
        {
            pCmd->x = toCmdPos(downState[0] + (int)diff.x);
            pCmd->y = toCmdPos(downState[1] + (int)diff.y);
        }
        else if (dragId == 1)
        {
            Vector2 p{(float)pCmd->x, (float)pCmd->y};
            float dist = Vector2::Distance(p, mousePos);
            pCmd->radius = downState[0] + ((int)dist - downState[1]);
        }
    }
    else if (dynamic_cast<sTextureCmdLINE*>(cmd))
    {
        sTextureCmdLINE* pCmd = (sTextureCmdLINE*)cmd;
        if (dragId == 0)
        {
            pCmd->x1 = toCmdPos(downState[0] + (int)diff.x);
            pCmd->y1 = toCmdPos(downState[1] + (int)diff.y);
        }
        else if (dragId == 1)
        {
            pCmd->x2 = toCmdPos(downState[0] + (int)diff.x);
            pCmd->y2 = toCmdPos(downState[1] + (int)diff.y);
        }
    }
    else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
    {
        updateTextEdit_RECT((sTextureCmdGRADIENT*)cmd, diff, mousePos, dragId, downState);
    }
    else if (dynamic_cast<sTextureCmdIMAGE*>(cmd))
    {
        updateTextEdit_RECT((sTextureCmdIMAGE*)cmd, diff, mousePos, dragId, downState);
    }

    workingTexture->bake(workingChannel);
}

void MainVC::load()
{
    vector<uint8_t> data;

    // Load the byte array
    ifstream fic("../../../DemoScene/res_data.h");
    char comma;
    while (!fic.eof())
    {
        int b;
        fic >> b;
        if (fic.eof()) break;
        fic >> comma;
        data.push_back((uint8_t)b);
    }
    fic.close();
    if (data.empty()) return;

    // Update our size label
    dataSize = (int)data.size();
    uiScreen.getChild<UILabel>("lblDataSize")->textComponent.text = to_string(dataSize) + " bytes";

    // Decompress
    int uncompressedSize;
    auto pDecompressedData = decompress(data.data(), (int)data.size(), uncompressedSize);
    data.clear();
    for (int i = 0; i < uncompressedSize; ++i)
    {
        data.push_back(pDecompressedData[i]);
    }
    delete[] pDecompressedData;

    readData = data.data();
    readPos = 0;

    // Load palette
    readBits(8);
    readBits(8);
    readBits(8);
    readBits(8);
    int colorCount = readBits(8);
    for (int i = 0; i < colorCount; ++i)
    {
        res_palColor color;
        color.x = (uint8_t)readBits(8);
        color.y = (uint8_t)readBits(8);
        color.z = (uint8_t)readBits(8);
        color.w = (uint8_t)readBits(8);
        res_palette.push_back(color);
    }

    // Deserialize
    bool bDone = false;
    while (!bDone)
    {
        auto b = readBits(8);
        switch (b)
        {
            case RES_END:
            {
                bDone = true;
                break;
            }
            case RES_IMG:
            {
                auto pTexture = new sTexture();
                pTexture->deserialize();
                pTexture->bake(CHANNEL_DIFFUSE);
                pTexture->bake(CHANNEL_NORMAL);
                pTexture->bake(CHANNEL_MATERIAL);

                auto pSelectBox = new UICheckBox();

                pSelectBox->rect.size = {128, 128};
                pSelectBox->pUserData = pTexture;
                pSelectBox->setStyle("selectTexture");
                pSelectBox->behavior = eUICheckBehavior::EXCLUSIVE;
                pSelectBox->onDoubleClick = [this](UIControl* c, const UIMouseEvent& e)
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

                uiTextures->add(pSelectBox);
                res_textures.push_back(pTexture);
                break;
            }
        }
    }
}

void MainVC::save()
{
    compressedData.clear();
    writePos = 0;

    write((int)res_textures.size(), 8);
    write(0, 8);
    write(0, 8);
    write(0, 8);

    // Palette
    updatePalette();
    write((int)res_palette.size(), 8);
    for (auto& color : res_palette)
    {
        write((int)color.x, 8);
        write((int)color.y, 8);
        write((int)color.z, 8);
        write((int)color.w, 8);
    }

    // Serialize textures
    for (auto texture : res_textures)
    {
        texture->serialize();
    }

    // End marker
    write(RES_END, 8);

    // Write padding at the end
    auto padding = 8 - writePos % 8;
    if (padding)
    {
        write(0, padding);
    }

    // Compress
    vector<uint8_t> data = compressedData;
    int compressedSize = 0;
    auto* pCompressedData = compress(data.data(), (int)data.size(), compressedSize);

    // Save the byte array
    dataSize = compressedSize;
    uiScreen.getChild<UILabel>("lblDataSize")->textComponent.text = to_string(dataSize) + " bytes";
    ofstream fic("../../../DemoScene/res_data.h");
    for (int i = 0; i < compressedSize; ++i)
    {
        fic << (int)pCompressedData[i] << ",";
    }
    fic.close();
    delete []pCompressedData;

    // Notify with a little animation
    uiSavedV.start({
        {1, 0},
        {0, 3, OTeleport}
    });
    uiSavedY.start({
        {uiSaved->rect.position.y - uiSaved->rect.size.y, .25f, OEaseOut},
        {uiSaved->rect.position.y - uiSaved->rect.size.y, 2.5f},
        {uiSaved->rect.position.y, .25f, OEaseIn},
    });
}
