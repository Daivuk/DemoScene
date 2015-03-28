#include <d3d11.h>
#include "D3Dcompiler.h"
#include "mem.h"
#include "compress.h"
#include "string.h"

HWND windowHandle;

// Device stuff
IDXGISwapChain*             swapChain;
ID3D11Device*               device;
ID3D11DeviceContext*        deviceContext;
ID3D11RenderTargetView*     renderTargetView;
ID3D11VertexShader*         vs2d;
ID3D11PixelShader*          ps2d;
ID3D11InputLayout*          il2d;
ID3D11Buffer*               viewProj2dBuffer;
ID3D11DepthStencilState*    pDs2D;
ID3D11RasterizerState*      pSr2D;
ID3D11BlendState*           pBs2D;
ID3D11SamplerState*         pSs2D;

const auto g_vs2d = "\
M m;\
S O\
{\
j p:V;\
g t:T;\
j c:C;\
};\
O vs_5_0(g p:P,g t:T,j c:C)\
{\
O o={mul(j(p.xy,0,1),m),t,c};\
r o;\
}";

const auto g_ps2d = "\
Y x;\
R s;\
j ps_5_0(j p:V,g t:T,j c:C):A\
{\
r x.D(s,t)*c;\
}";

#define RESOLUTION_W 1024
#define RESOLUTION_H 600

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

    // Centered position
    auto screenW = GetSystemMetrics(SM_CXSCREEN);
    auto screenH = GetSystemMetrics(SM_CYSCREEN);
    auto posX = (screenW - RESOLUTION_W) / 2;
    auto posY = (screenH - RESOLUTION_H) / 2;

    // Create the window
    windowHandle = CreateWindow("D", "",
                            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                            posX, posY, RESOLUTION_W, RESOLUTION_H,
                            nullptr, nullptr, nullptr, nullptr);
}

D3D_SHADER_MACRO Shader_Macros[] = {
    {"S", "struct"},
    {"M", "matrix"},
    {"g", "float2"},
    {"j", "float4"},
    {"P", "POSITION"},
    {"T", "TEXCOORD"},
    {"C", "COLOR"},
    {"Y", "Texture2D"},
    {"D", "Sample"},
    {"R", "SamplerState"},
    {"V", "SV_POSITION"},
    {"A", "SV_TARGET"},
    {"r", "return"},
    {nullptr, nullptr},
};

ID3DBlob* gfx_compileShader(const char* shader, const char stype)
{
    // Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
    char profile[] = "ps_5_0";
    profile[0] = stype;
    ID3DBlob* shaderBlob = nullptr;
#ifdef _DEBUG
    ID3DBlob* errorBlob = nullptr;
#endif

    auto shaderCnt = shader;
    while (*shaderCnt++);
    int size = shaderCnt - shader;

#ifdef _DEBUG
    HRESULT hr = D3DCompile(shader, size, nullptr, Shader_Macros, nullptr, profile, profile, D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, &shaderBlob, &errorBlob);
    if (errorBlob)
    {
        auto pError = (char*)errorBlob->GetBufferPointer();
        int tmp;
        tmp = 5;
    }
#else
    D3DCompile(shader, size, nullptr, Shader_Macros, nullptr, profile, profile, D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, nullptr);
#endif
    return shaderBlob;
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
    auto viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)RESOLUTION_W, (float)RESOLUTION_H);
    deviceContext->RSSetViewports(1, &viewport);

    // Compile shaders. It's smaller that way than including pre-compiled files. They put a lot of crap inside.
    // We can always compress text!
    auto vs2db = gfx_compileShader(g_vs2d, 'v');
    auto ps2db = gfx_compileShader(g_ps2d, 'p');

    device->CreateVertexShader(vs2db->GetBufferPointer(), vs2db->GetBufferSize(), nullptr, &vs2d);
    device->CreatePixelShader(ps2db->GetBufferPointer(), ps2db->GetBufferSize(), nullptr, &ps2d);

    // Create input layouts
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    device->CreateInputLayout(layout, 3, vs2db->GetBufferPointer(), vs2db->GetBufferSize(), &il2d);

    // Make sure to render triangles
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create uniforms
    {
        const float matrix[] {
            2.f / RESOLUTION_W, 0.f,                    0.f,               -1.f,
            0.f,               -2.f / RESOLUTION_H,     0.f,                1.f,
            0.f,                0.f,                   -0.000500500493f,    0.5f,
            0.f,                0.f,                    0.f,                1.f};
        D3D11_BUFFER_DESC cbDesc = CD3D11_BUFFER_DESC(64, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        D3D11_SUBRESOURCE_DATA initData{matrix, 0, 0};
        device->CreateBuffer(&cbDesc, &initData, &viewProj2dBuffer);
    }

    // 2D Depth state
    D3D11_DEPTH_STENCIL_DESC depthDesc{
        false,
        D3D11_DEPTH_WRITE_MASK_ZERO,
        D3D11_COMPARISON_LESS,
        false,
        0xFF,
        0xFF,
        {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS},
        {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_DECR, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS}
    };
    device->CreateDepthStencilState(&depthDesc, &pDs2D);

    // 2D Rasterizer state
    D3D11_RASTERIZER_DESC rasterizerDesc{
        D3D11_FILL_SOLID,
        D3D11_CULL_NONE,
        false,
        0,
        0.f,
        0.f,
        false,
        false,
        false,
        false
    };
    device->CreateRasterizerState(&rasterizerDesc, &pSr2D);

    // 2D Blend state
    D3D11_BLEND_DESC blendDesc{
        FALSE,
        FALSE,
        {{
            TRUE,
            D3D11_BLEND_ONE,
            D3D11_BLEND_INV_SRC_ALPHA,
            D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE,
            D3D11_BLEND_ONE,
            D3D11_BLEND_OP_ADD,
            D3D10_COLOR_WRITE_ENABLE_ALL
        }, {0}, {0}, {0}, {0}, {0}, {0}, {0}}
    };
    device->CreateBlendState(&blendDesc, &pBs2D);

    // 2D Sampler state
    D3D11_SAMPLER_DESC samplerDesc{
        D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_WRAP,
        0.f,
        1,
        D3D11_COMPARISON_ALWAYS,
        {0, 0, 0, 0},
        0,
        D3D11_FLOAT32_MAX
    };
    device->CreateSamplerState(&samplerDesc, &pSs2D);
}

void gfx_endFrame()
{
    // Swap the buffer!
    swapChain->Present(1, 0);
}

void gfx_setup2d()
{
    // Set render states
    deviceContext->OMSetDepthStencilState(pDs2D, 1);
    deviceContext->RSSetState(pSr2D);
    deviceContext->OMSetBlendState(pBs2D, NULL, 0xffffffff);
    deviceContext->PSSetSamplers(0, 1, &pSs2D);

    // Bind the shaders
    deviceContext->IASetInputLayout(il2d);
    deviceContext->VSSetShader(vs2d, nullptr, 0);
    deviceContext->PSSetShader(ps2d, nullptr, 0);

    // Bind the matrix
    deviceContext->VSSetConstantBuffers(0, 1, &viewProj2dBuffer);
}

void gfx_beginFrame()
{
    const float color[] = {0, 0, 0, 1};
    deviceContext->ClearRenderTargetView(renderTargetView, color);
}
