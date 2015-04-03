#pragma once

#include "onut.h"
using namespace onut;

class resTexture
{
public:
    void generate();

    Texture *pTexture = nullptr;
    int w = 64;
    int h = 64;

    UICheckBox* pChk = nullptr;
};
