#pragma once

#include "resources.h"
#include <map>
using namespace onut;

class MainVC
{
public:
    struct sSelectTextureInfo
    {
        size_t index;
        UICheckBox* selectBox;
        sTexture* texture;
    };

    struct sSelectCmdInfo
    {
        size_t index;
        UICheckBox* selectBox;
        sTextureCmd* cmd;
    };

    MainVC();

    void update();
    void render();

    void closeAllViews();
    sSelectTextureInfo getSelectedTexture() const;
    sSelectCmdInfo getSelectedCmd() const;
    void repopulateCmds();
    void insertCmd(sTextureCmd* pCmd, UIControl* pCtrl);
    void addCmd(sTextureCmd* pCmd, UIControl* pCtrl);
    void buildUIForTexture();
    void hookCmd(sTextureCmd* pCmd, UIControl* pCtrl);

    void showColorPicker(const Color& color, function<void(const Color&)> callback);
    void updateColorPickerValues();

public:
    UIContext uiContext;
    UIControl uiScreen;

    UIControl* uiTextures;
    UIControl* uiTexture;
    UIControl* uiInspectorTextures;
    UIControl* uiInspectorTexture;
    UIControl* uiColorPicker;

    sTexture* workingTexture;

    map<eRES_CMD, UICheckBox*> cmdControls;
    UIControl* uiCmdStack;
    UITextBox* uiTextureW;
    UITextBox* uiTextureH;
    UIControl* pnlTexture;

    function<void(const Color&)> colorPickerCallback;
};
