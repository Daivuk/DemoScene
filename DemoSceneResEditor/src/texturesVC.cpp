#include "texturesVC.h"
#include "mainVC.h"
#include "resTexture.h"

extern MainVC* pMainVC;

using namespace std;

static const Color g_guideColor = OColorHex(3399ff);
static const Color g_toolBtnHoverColor = OColorHex(3e3e40);
static const Color g_toolBtnDownColor = OColorHex(007acc);

TexturesVC::TexturesVC()
    : m_uiScreen("../../assets/ui/textures.json")
{
    // create UI styles
    pMainVC->m_uiContext.addStyle<UICheckBox>("chkTexture", [this](const onut::UICheckBox* pCheckBox, const onut::sUIRect& rect)
    {
        auto state = pCheckBox->getState(pMainVC->m_uiContext);
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
        auto resT = (resTexture*)pCheckBox->pUserData;
        orect.x += 2;
        orect.y += 2;
        orect.z -= 4;
        orect.w -= 4;
        OSB->drawRectWithUVs(OGetTexture("dottedLine.png"), orect, {0, 0, orect.z / 16, orect.w / 16}, g_toolBtnHoverColor);
        OSB->drawRect(resT->pTexture, orect);
    });

    m_pChkTemplate = m_uiScreen.getChild<UICheckBox>("chkTexture");
    m_pChkTemplate->retain();
    m_pChkTemplate->behavior = eUICheckBehavior::CHK_OPTIONAL;

    m_pTexturesView = m_uiScreen.getChild("texturesView");
    m_pTexturesView->remove(m_pChkTemplate);

    // hook events
    m_uiScreen.getChild("btnNew")->onClick = bind(&TexturesVC::onNew, this, placeholders::_1, placeholders::_2);
    m_uiScreen.getChild("btnEdit")->onClick = bind(&TexturesVC::onEdit, this, placeholders::_1, placeholders::_2);
    m_uiScreen.getChild("btnDelete")->onClick = bind(&TexturesVC::onDelete, this, placeholders::_1, placeholders::_2);

    pMainVC->m_pMainView->add(&m_uiScreen);
}

void TexturesVC::update()
{
    // relayout the checkboxes
    sUIRect rect{{0, 0}, {136, 136}};
    for (auto pRes : textures)
    {
        pRes->pChk->rect = rect;
        rect.position.x += rect.size.x;
    }
}

void TexturesVC::onNew(UIControl* pControl, const UIMouseEvent& e)
{
    auto pRes = new resTexture();
    textures.push_back(pRes);
    pRes->pChk = new UICheckBox(*m_pChkTemplate);
    pRes->pChk->pUserData = pRes;
    m_pTexturesView->add(pRes->pChk);
    pRes->generate();
}

void TexturesVC::onEdit(UIControl* pControl, const UIMouseEvent& e)
{
}

void TexturesVC::onDelete(UIControl* pControl, const UIMouseEvent& e)
{
    for (int i = 0; i < (int)textures.size(); ++i)
    {
        if (textures[i]->pChk->getIsChecked())
        {
            if (i + 1 < (int)textures.size())
            {
                textures[i + 1]->pChk->setIsChecked(true);
            }
            else if (i > 0)
            {
                textures[i - 1]->pChk->setIsChecked(true);
            }
            textures[i]->pChk->release();
            delete textures[i];
            textures.erase(textures.begin() + i);
            break;
        }
    }
}
