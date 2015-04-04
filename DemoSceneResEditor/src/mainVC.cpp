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

    uiPnlTexture = uiScreen.getChild("pnlTexture");
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
        workingTexture->bake();
    };
    uiTextureH->onTextChanged = [this](UITextBox* c, const UITextBoxEvent& e)
    {
        auto d = uiTextureH->getInt();
        pnlTexture->rect.size.y = (float)d;
        workingTexture->h = d;
        workingTexture->bake();
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
    else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
    {
        sTextureCmdBEVEL* pCmd = (sTextureCmdBEVEL*)cmd;

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

        auto size = pCtrl->getChild<UITextBox>("txtBevel");
        size->setInt(pCmd->bevel);
        size->onTextChanged = [this, pCmd](UITextBox* pCtrl, const UITextBoxEvent& evt)
        {
            pCmd->bevel = pCtrl->getInt();
            workingTexture->bake();
        };
    }
    else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
    {
        sTextureCmdCIRCLE* pCmd = (sTextureCmdCIRCLE*)cmd;

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
    else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
    {
        sTextureCmdBEVEL_CIRCLE* pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;

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

        auto size = pCtrl->getChild<UITextBox>("txtBevel");
        size->setInt(pCmd->bevel);
        size->onTextChanged = [this, pCmd](UITextBox* pCtrl, const UITextBoxEvent& evt)
        {
            pCmd->bevel = pCtrl->getInt();
            workingTexture->bake();
        };
    }
    else if (dynamic_cast<sTextureCmdLINE*>(cmd))
    {
        sTextureCmdLINE* pCmd = (sTextureCmdLINE*)cmd;

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

        auto size = pCtrl->getChild<UITextBox>("txtSize");
        size->setInt(pCmd->size);
        size->onTextChanged = [this, pCmd](UITextBox* pCtrl, const UITextBoxEvent& evt)
        {
            pCmd->size = pCtrl->getInt();
            workingTexture->bake();
        };
    }
    else if (dynamic_cast<sTextureCmdNORMAL_MAP*>(cmd))
    {
        sTextureCmdNORMAL_MAP* pCmd = (sTextureCmdNORMAL_MAP*)cmd;
    }
    else if (dynamic_cast<sTextureCmdGRADIENT*>(cmd))
    {
        sTextureCmdGRADIENT* pCmd = (sTextureCmdGRADIENT*)cmd;

        auto colorPicker = pCtrl->getChild<UIPanel>("colorFrom");
        colorPicker->color = sUIColor{pCmd->color1.x, pCmd->color1.y, pCmd->color1.z, pCmd->color1.w};
        colorPicker->onClick = [this, pCmd, pCtrl, colorPicker](UIControl* pCtrl, const UIMouseEvent& evt)
        {
            showColorPicker(pCmd->color1, [this, pCmd, pCtrl, colorPicker](const Color& color){
                pCmd->color1 = color;
                colorPicker->color = sUIColor{color.x, color.y, color.z, color.w};
                workingTexture->bake();
            });
        };

        auto color2Picker = pCtrl->getChild<UIPanel>("colorTo");
        color2Picker->color = sUIColor{pCmd->color2.x, pCmd->color2.y, pCmd->color2.z, pCmd->color2.w};
        color2Picker->onClick = [this, pCmd, pCtrl, color2Picker](UIControl* pCtrl, const UIMouseEvent& evt)
        {
            showColorPicker(pCmd->color2, [this, pCmd, pCtrl, color2Picker](const Color& color){
                pCmd->color2 = color;
                color2Picker->color = sUIColor{color.x, color.y, color.z, color.w};
                workingTexture->bake();
            });
        };
    }
}

void MainVC::insertCmd(sTextureCmd* pCmd, UIControl* pCtrl)
{
    pCmd->texture = workingTexture;
    pCtrl->pUserData = pCmd;
    auto selected = getSelectedCmd();
    auto index = selected.index;
    if (index < workingTexture->cmds.size()) ++index;
    uiCmdStack->insertAt(pCtrl, index);
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
                if (selectedCmd.index < workingTexture->cmds.size() - 1)
                {
                    selectedCmd.selectBox->retain();
                    selectedCmd.selectBox->remove();
                    workingTexture->cmds.erase(workingTexture->cmds.begin() + selectedCmd.index);
                    ++selectedCmd.index;

                    uiCmdStack->insertAt(selectedCmd.selectBox, selectedCmd.index);
                    workingTexture->cmds.insert(workingTexture->cmds.begin() + selectedCmd.index, selectedCmd.cmd);

                    selectedCmd.selectBox->release();

                    workingTexture->bake();
                }
            }
            else if (OInput->isStateJustDown(DIK_UP) &&
                     OInput->isStateDown(DIK_LCONTROL))
            {
                if (selectedCmd.index > 0)
                {
                    selectedCmd.selectBox->retain();
                    selectedCmd.selectBox->remove();
                    workingTexture->cmds.erase(workingTexture->cmds.begin() + selectedCmd.index);
                    --selectedCmd.index;

                    uiCmdStack->insertAt(selectedCmd.selectBox, selectedCmd.index);
                    workingTexture->cmds.insert(workingTexture->cmds.begin() + selectedCmd.index, selectedCmd.cmd);

                    selectedCmd.selectBox->release();

                    workingTexture->bake();
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
        }
    }
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
        sTextureCmdRECT* pCmd = (sTextureCmdRECT*)cmd;
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
    }
    else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
    {
        sTextureCmdBEVEL* pCmd = (sTextureCmdBEVEL*)cmd;
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
    }
    else if (dynamic_cast<sTextureCmdCIRCLE*>(cmd))
    {
        sTextureCmdCIRCLE* pCmd = (sTextureCmdCIRCLE*)cmd;
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
    else if (dynamic_cast<sTextureCmdBEVEL_CIRCLE*>(cmd))
    {
        sTextureCmdBEVEL_CIRCLE* pCmd = (sTextureCmdBEVEL_CIRCLE*)cmd;
        Vector2 p{(float)pCmd->x, (float)pCmd->y};
        float dist = Vector2::DistanceSquared(p, pos);
        float radius = (float)max<>(8, pCmd->radius);
        if (dist < radius * radius)
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
        sTextureCmdGRADIENT* pCmd = (sTextureCmdGRADIENT*)cmd;
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
    }

    return -1;
}

int toCmdPos(int pos)
{
    pos = max<>(-32, pos);
    pos = min<>(988, pos);
    return pos / 4 * 4;
}

void MainVC::updateTextureEdit(const Vector2& diff, const Vector2& mousePos)
{
    auto cmd = getSelectedCmd().cmd;
    if (!cmd) return;
    if (dragId == -1) return;

    if (dynamic_cast<sTextureCmdRECT*>(cmd))
    {
        sTextureCmdRECT* pCmd = (sTextureCmdRECT*)cmd;
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
    else if (dynamic_cast<sTextureCmdBEVEL*>(cmd))
    {
        sTextureCmdBEVEL* pCmd = (sTextureCmdBEVEL*)cmd;
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
        sTextureCmdGRADIENT* pCmd = (sTextureCmdGRADIENT*)cmd;
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

    workingTexture->bake();
}

#include <fstream>

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

    // Deserialize
    for (size_t i = 4; i < data.size(); ++i)
    {
        auto b = data[i];
        switch (b)
        {
            case RES_IMG:
            {
                auto pTexture = new sTexture();
                i += pTexture->deserialize(data.data() + i + 1);
                pTexture->bake();

                auto pSelectBox = new UICheckBox();

                pSelectBox->rect.size = {128, 128};
                pSelectBox->pUserData = pTexture;
                pSelectBox->setStyle("selectTexture");
                pSelectBox->behavior = eUICheckBehavior::EXCLUSIVE;

                uiTextures->add(pSelectBox);
                res_textures.push_back(pTexture);
                break;
            }
        }
    }
}

void MainVC::save()
{
    vector<uint8_t> data;

    data.push_back((uint8_t)res_textures.size()); // Texture count
    data.push_back(0); // Mesh count
    data.push_back(0); // Model count
    data.push_back(0); // Camera count

    // Serialize textures
    for (auto texture : res_textures)
    {
        texture->serialize(data);
    }

    // Compress
    // ... todo

    // Save the byte array
    ofstream fic("../../../DemoScene/res_data.h");
    for (auto b : data)
    {
        fic << (int)b << ",";
    }
    fic.close();

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
