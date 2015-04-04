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

    int pickOnTexture(const Vector2& pos, int* out_downState);
    void updateTextureEdit(const Vector2& diff, const Vector2& mousePos);

    void load();
    void save();

public:
    UIContext uiContext;
    UIControl uiScreen;

    UIControl* uiTextures;
    UIControl* uiTexture;
    UIControl* uiPnlTexture;
    UIControl* uiInspectorTextures;
    UIControl* uiInspectorTexture;
    UIControl* uiColorPicker;

    sTexture* workingTexture;

    map<eRES_CMD, UICheckBox*> cmdControls;
    UIControl* uiCmdStack;
    UITextBox* uiTextureW;
    UITextBox* uiTextureH;
    UIControl* pnlTexture;
    UIControl* uiSaved;
    OAnimf uiSavedY;
    OAnimi uiSavedV;

    int dragId = -1;
    int downState[4];
    Vector2 onDownMousePos;
    Vector2 lastDragDiff;
    bool bShowGrid = true;
    OAnimf cmdStackOffset = 0.f;

    int dataSize = 0;

    function<void(const Color&)> colorPickerCallback;
};
