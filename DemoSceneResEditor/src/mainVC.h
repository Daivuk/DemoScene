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

    void load();
    void save();

    // Generic state
    void closeAllViews();
    sSelectCmdInfo getSelectedCmd() const;
    void showColorPicker(const res_palColor& color, function<void(const res_palColor&)> callback);
    void updateColorPickerValues();

    // Textures
    sSelectTextureInfo getSelectedTexture() const;
    void repopulateCmds();
    void buildUIForTexture();

    // Commands
    void insertCmd(sTextureCmd* pCmd, UIControl* pCtrl);
    void addCmd(sTextureCmd* pCmd, UIControl* pCtrl);
    void hookCmd(sTextureCmd* pCmd, UIControl* pCtrl);
    int pickOnTexture(const Vector2& pos, int* out_downState);
    void updateTextureEdit(const Vector2& diff, const Vector2& mousePos);
    void shiftTextureReferences(int index, int inc);

public:
    UIContext uiContext;
    UIControl uiScreen;

    UIControl* uiTextures;
    UIControl* uiTexture;
    UIControl* uiPnlTexture;
    UIControl* uiInspectorTextures;
    UIControl* uiInspectorTexture;
    UIControl* uiColorPicker;
    UIControl* uiColorPickerPreviousContainer;
    UIControl* uiColorPickerPreviousSample;

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

    function<void(const res_Color&)> colorPickerCallback;
};
