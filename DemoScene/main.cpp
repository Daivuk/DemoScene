extern "C" int _fltused = 0x9875; // has taken from "stub.c" in the CRT sources.

#include <windows.h>
#include "ds_mem.h"
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
    // Setup frame
    gfx_beginFrame();

    // Draw 3D
    if (res_cameraCount)
    {
        gfx_setup3d(res_currentCamera);
        for (int i = 0; i < res_modelCount; ++i)
        {
            gfx_drawModel(res_models[i]);
        }
    }

    // Draw 2D
    gfx_setup2d();

    const float rect[] {64, 64, 128, 256};
    spr_draw(res_textures[0].view, rect, GFX_WHITE);
    //const float rect2[] {64 + 128 + 64, 64, 128, 256};
    //spr_draw(res_textures[2].view, rect2, GFX_WHITE);

    // Flush and flip
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
