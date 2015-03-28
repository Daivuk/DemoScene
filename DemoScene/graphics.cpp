#include <d3d11.h>
#include "mem.h"

HWND windowHandle;

// Device stuff
IDXGISwapChain*             swapChain;
ID3D11Device*               device;
ID3D11DeviceContext*        deviceContext;
ID3D11RenderTargetView*     renderTargetView;

LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_DESTROY ||
        msg == WM_CLOSE)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(handle, msg, wparam, lparam);
}

void createWindow()
{
    // Define window style
    WNDCLASS wc;
    mem_zero(&wc, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WinProc;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "D";
    RegisterClass(&wc);

    POINT resolution{800, 600};

    // Centered position
    auto screenW = GetSystemMetrics(SM_CXSCREEN);
    auto screenH = GetSystemMetrics(SM_CYSCREEN);
    auto posX = (screenW - resolution.x) / 2;
    auto posY = (screenH - resolution.y) / 2;

    // Create the window
    windowHandle = CreateWindow("D", "",
                            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                            posX, posY, resolution.x, resolution.y,
                            nullptr, nullptr, nullptr, nullptr);
}

void gfx_init()
{
    // Create the window
    createWindow();

    // Define our swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    mem_zero(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = true;

    // Create the swap chain, device and device context
    D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
#if _DEBUG
        D3D11_CREATE_DEVICE_DEBUG,
#else
        0,
#endif
        nullptr, 0, D3D11_SDK_VERSION,
        &swapChainDesc, &swapChain,
        &device, nullptr, &deviceContext);

    // Create render target
    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

    // Bind render target
    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
}

void gfx_endFrame()
{
    // Swap the buffer!
    swapChain->Present(1, 0);
}
