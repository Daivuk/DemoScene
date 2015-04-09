#include <Windows.h>
#include "onut.h"
#include "texturesVC.h"
#include "mainVC.h"

enum class eState
{
    MAIN_MENU,
    TEXTURES
};

eState g_state = eState::TEXTURES;

MainVC* pMainVC;

int CALLBACK WinMain(
    _In_  HINSTANCE hInstance,
    _In_  HINSTANCE hPrevInstance,
    _In_  LPSTR lpCmdLine,
    _In_  int nCmdShow
    )
{
    OSettings->setGameName("Demo Scene Resources Editor");
    OSettings->setResolution({1280, 720});

    onut::run([]
    {
        pMainVC = new MainVC();

        OWindow->onWrite = [](char c){pMainVC->uiContext.write(c); };
        OWindow->onKey = [](uintptr_t key)
        {
            //if (!g_pDocument->isBusy())
            {
                pMainVC->uiContext.keyDown(key);
                //if (!dynamic_cast<onut::UITextBox*>(pMainVC->m_uiContext.getFocusControl()))
                //{
                //    //checkShortCut(key);
                //    g_pDocument->onKeyDown(key);
                //}
            }
        };
    },
        []
    {
        pMainVC->uiContext.resize({OScreenWf, OScreenHf});

        switch (g_state)
        {
            case eState::TEXTURES: pMainVC->update(); break;
        }
    },
        []
    {
        ORenderer->clear(Color::Black);
        switch (g_state)
        {
            case eState::TEXTURES: pMainVC->render(); break;
        }
    });
}
