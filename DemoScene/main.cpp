extern "C" int _fltused = 0x9875; // has taken from "stub.c" in the CRT sources.

#include <windows.h>
#include "mem.h"
#include "graphics.h"
#include "res.h"
#include "sprite.h"

void update()
{
    static DWORD lastT = GetTickCount();
    auto t = GetTickCount();
    float dt = (float)(t - lastT) / 1000.f;
    lastT = t;
}

void draw()
{
    gfx_beginFrame();
    gfx_setup2d();

    const float rect[] {100, 200, 300, 400};
    spr_draw(res_texWhite, rect);

    spr_flush();
    gfx_endFrame();
}

void main()
{
    mem_init(); // Init heap memory
    gfx_init(); // Init DX11
    spr_init(); // Init sprite batch
    res_load(); // Load resources

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
