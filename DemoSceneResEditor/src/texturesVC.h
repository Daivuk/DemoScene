#pragma once

#include "onut.h"
#include <vector>
using namespace std;

using namespace onut;

class resTexture;

class TexturesVC
{
public:
    TexturesVC();

    void update();

private:
    void onNew(UIControl*, const UIMouseEvent&);
    void onEdit(UIControl*, const UIMouseEvent&);
    void onDelete(UIControl*, const UIMouseEvent&);

    UIControl m_uiScreen;
    UICheckBox* m_pChkTemplate;
    UIControl* m_pTexturesView;

    vector<resTexture*> textures;
};
