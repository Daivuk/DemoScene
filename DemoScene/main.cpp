#include <windows.h>

#include "graphics.h"

void update()
{
    static DWORD lastT = GetTickCount();
    auto t = GetTickCount();
    float dt = (float)(t - lastT) / 1000.f;
    lastT = t;
}

void draw()
{
    gfx_endFrame();
}

//int CALLBACK WinMain(
//    _In_  HINSTANCE hInstance,
//    _In_  HINSTANCE hPrevInstance,
//    _In_  LPSTR lpCmdLine,
//    _In_  int nCmdShow)
void main()
{
    gfx_init();

    // Main loop
    MSG msg;
    while (true)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }

        update();
        draw();
    }

    ExitProcess(0);
}
