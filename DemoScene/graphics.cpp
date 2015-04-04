#include <d3d11.h>
#include "D3Dcompiler.h"
#include "ds_mem.h"
#include "compress.h"
#include "string.h"
#include "graphics.h"
#include "res.h"
#include "mat.h"

#if _DEBUG
#include <cassert>
#endif

HWND windowHandle;

// Device stuff
IDXGISwapChain*             swapChain;
ID3D11Device*               device;
ID3D11DeviceContext*        deviceContext;
ID3D11RenderTargetView*     renderTargetView;

ID3D11VertexShader*         vs2d;
ID3D11PixelShader*          ps2d;
ID3D11VertexShader*         vs3d;
ID3D11PixelShader*          ps3d;
ID3D11InputLayout*          il2d;
ID3D11InputLayout*          il3d;

ID3D11Buffer*               viewProj2dBuffer;
ID3D11Buffer*               proj3dBuffer;
ID3D11Buffer*               view3dBuffer;
ID3D11Buffer*               world3dBuffer;
ID3D11DepthStencilState*    pDs2D;
ID3D11RasterizerState*      pSr2D;
ID3D11BlendState*           pBs2D;
ID3D11SamplerState*         pSs2D;
ID3D11DepthStencilState*    pDs3D;
ID3D11RasterizerState*      pSr3D;
ID3D11BlendState*           pBs3D;
ID3D11SamplerState*         pSs3D;

static const unsigned int stride = (3 + 3 + 2) * 4;
static const unsigned int offset = 0;

const auto g_vs2d = 
"\
B mm:G(b0){M m;}\
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

const auto g_ps2d = 
"\
Y x:G(t0);\
R s:G(s0);\
j ps_5_0(j p:V,g t:T,j c:C):A\
{\
r x.D(s,t)*c;\
}";

const auto g_vs3d =
"\
B mm:G(b0){M m;}\
B vv:G(b1){M v;}\
B ww:G(b2){M w;}\
S O\
{\
j p:V;\
h n:N;\
g t:T;\
h wp:T1;\
};\
O vs_5_0(h p:P,h n:N,g t:T)\
{\
O o;\
o.wp=mul(j(p,1),w).xyz;\
o.p=mul(mul(j(o.wp,1),v),m);\
o.n=mul(j(n,0),w).xyz;\
o.t=t;\
r o;\
}";

const auto g_ps3d =
"\
Y x0:G(t0);\
Y x1:G(t1);\
R s:G(s0);\
j ps_5_0(j p:V,h n:N,g t:T,h wp:T1):A\
{\
j di=x0.D(s,t);\
h no=x1.D(s,t).xyz;\
h norm=no*2-1;\
norm=h(norm.x,-norm.z,-norm.y);\
norm=normalize(norm);\
f dt=distance(h(-32,384,256),wp);\
dt=saturate(1-dt/1024);\
dt*=saturate(dot(normalize(h(-32,384,256)-wp),norm));\
r di*dt*1.5;\
}";

float gfxData[] 
{
    // viewProj2D matrix
    2.f / RESOLUTION_W, 0.f,                    0.f,               -1.f,
    0.f,               -2.f / RESOLUTION_H,     0.f,                1.f,
    0.f,                0.f,                   -0.000500500493f,    0.5f,
    0.f,                0.f,                    0.f,                1.f,

    // Identity matrix
         0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,

    // black
    0.f, 0.f, 0.f, 1.f,

    // white
    1.f, 1.f, 1.f,

    // Porjection matrix
    1.18930638f, 0.f, 0.f, 0.f,
    0.f, 2.11432242f, 0.f, 0.f,
    0.f, 0.f, -1.00001001f, -0.100001000f,
    0.f, 0.f, -1.f, 0.f,
};
//+ [0]	{m128_f32 = 0x005ec7d0 {1.18930638, 0.000000000, 0.000000000, 0.000000000} m128_u64 = 0x005ec7d0 {1066941233, ...} ...}	__m128
//+ [1]	{m128_f32 = 0x005ec7e0 {0.000000000, 2.11432242, 0.000000000, 0.000000000} m128_u64 = 0x005ec7e0 {4613745468130721792, ...} ...}	__m128
//+ [2]	{m128_f32 = 0x005ec7f0 {0.000000000, 0.000000000, -1.00001001, -1.00000000} m128_u64 = 0x005ec7f0 {0, 13799029261476036692} ...}	__m128
//+ [3]	{m128_f32 = 0x005ec800 {0.000000000, 0.000000000, -0.100001000, 0.000000000} m128_u64 = 0x005ec800 {0, 3184315731} ...}	__m128

D3D_SHADER_MACRO Shader_Macros[] = {
    {"S", "struct"},
    {"M", "matrix"},
    {"f", "float"},
    {"g", "float2"},
    {"h", "float3"},
    {"j", "float4"},
    {"P", "POSITION"},
    {"N", "NORMAL"},
    {"T", "TEXCOORD"},
    {"T1", "TEXCOORD1"},
    {"C", "COLOR"},
    {"Y", "Texture2D"},
    {"D", "Sample"},
    {"R", "SamplerState"},
    {"V", "SV_POSITION"},
    {"A", "SV_TARGET"},
    {"r", "return"},
    {"B", "cbuffer"},
    {"G", "register"},
    {nullptr, nullptr},
};

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
        assert(false);
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
#if _DEBUG
    backBuffer->Release();
#endif

    // Bind render target
    deviceContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
    auto viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)RESOLUTION_W, (float)RESOLUTION_H);
    deviceContext->RSSetViewports(1, &viewport);

    // Compile shaders. It's smaller that way than including pre-compiled files. They put a lot of crap inside.
    // We can always compress text!
    auto vs2db = gfx_compileShader(g_vs2d, 'v');
    auto ps2db = gfx_compileShader(g_ps2d, 'p');
    auto vs3db = gfx_compileShader(g_vs3d, 'v');
    auto ps3db = gfx_compileShader(g_ps3d, 'p');

    auto vs2dPtr = vs2db->GetBufferPointer();
    auto vs2dSize = vs2db->GetBufferSize();
    auto vs3dPtr = vs3db->GetBufferPointer();
    auto vs3dSize = vs3db->GetBufferSize();

    device->CreateVertexShader(vs2dPtr, vs2dSize, nullptr, &vs2d);
    device->CreatePixelShader(ps2db->GetBufferPointer(), ps2db->GetBufferSize(), nullptr, &ps2d);
    device->CreateVertexShader(vs3dPtr, vs3dSize, nullptr, &vs3d);
    device->CreatePixelShader(ps3db->GetBufferPointer(), ps3db->GetBufferSize(), nullptr, &ps3d);

    // Create input layouts
    {
        D3D11_INPUT_ELEMENT_DESC layout[3];
        mem_zero(layout, sizeof(D3D11_INPUT_ELEMENT_DESC) * 3);
        layout->SemanticName = "POSITION";
        layout->Format = DXGI_FORMAT_R32G32_FLOAT;
        layout->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        (layout + 1)->SemanticName = "TEXCOORD";
        (layout + 1)->Format = DXGI_FORMAT_R32G32_FLOAT;
        (layout + 1)->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        (layout + 1)->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        (layout + 2)->SemanticName = "COLOR";
        (layout + 2)->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        (layout + 2)->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        (layout + 2)->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        device->CreateInputLayout(layout, 3, vs2dPtr, vs2dSize, &il2d);
    }
    {
        D3D11_INPUT_ELEMENT_DESC layout[3];
        mem_zero(layout, sizeof(D3D11_INPUT_ELEMENT_DESC) * 3);
        layout->SemanticName = "POSITION";
        layout->Format = DXGI_FORMAT_R32G32B32_FLOAT;
        layout->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        (layout + 1)->SemanticName = "NORMAL";
        (layout + 1)->Format = DXGI_FORMAT_R32G32B32_FLOAT;
        (layout + 1)->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        (layout + 1)->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        (layout + 2)->SemanticName = "TEXCOORD";
        (layout + 2)->Format = DXGI_FORMAT_R32G32_FLOAT;
        (layout + 2)->AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        (layout + 2)->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        device->CreateInputLayout(layout, 3, vs3dPtr, vs3dSize, &il3d);
    }

    // Make sure to render triangles
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create uniforms
    {
        D3D11_BUFFER_DESC cbDesc = CD3D11_BUFFER_DESC(64, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        D3D11_SUBRESOURCE_DATA initData{GFX_VIEWPROJ2D, 0, 0};
        device->CreateBuffer(&cbDesc, &initData, &viewProj2dBuffer);
    }
    {
        D3D11_BUFFER_DESC cbDesc = CD3D11_BUFFER_DESC(64, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        D3D11_SUBRESOURCE_DATA initData{GFX_PROJ3D, 0, 0};
        device->CreateBuffer(&cbDesc, &initData, &proj3dBuffer);
    }
    {
        D3D11_BUFFER_DESC cbDesc = CD3D11_BUFFER_DESC(64, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        D3D11_SUBRESOURCE_DATA initData{GFX_IDENTITY, 0, 0};
        device->CreateBuffer(&cbDesc, &initData, &view3dBuffer);
        device->CreateBuffer(&cbDesc, &initData, &world3dBuffer);
    }

    // 2D Depth state
    {
        D3D11_DEPTH_STENCIL_DESC depthDesc;
        mem_zero(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        device->CreateDepthStencilState(&depthDesc, &pDs2D);
    }

    // 2D Rasterizer state
    {
        D3D11_RASTERIZER_DESC rasterizerDesc;
        mem_zero(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState(&rasterizerDesc, &pSr2D);
    }

    // 2D Blend state
    {
        D3D11_BLEND_DESC blendDesc;
        mem_zero(&blendDesc, sizeof(D3D11_BLEND_DESC));
        blendDesc.RenderTarget->BlendEnable = TRUE;
        blendDesc.RenderTarget->SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget->RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&blendDesc, &pBs2D);
    }

    // 2D Sampler state
    {
        D3D11_SAMPLER_DESC samplerDesc;
        mem_zero(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        device->CreateSamplerState(&samplerDesc, &pSs2D);
    }

    // 3D Depth state
    {
        D3D11_DEPTH_STENCIL_DESC depthDesc;
        mem_zero(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
        //true,
        //    D3D11_DEPTH_WRITE_MASK_ALL,
        //    D3D11_COMPARISON_LESS,
        //    false,
        //    D3D11_DEFAULT_STENCIL_READ_MASK,
        //    D3D11_DEFAULT_STENCIL_WRITE_MASK,
        //    {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS},
        //    {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS}
        device->CreateDepthStencilState(&depthDesc, &pDs3D);
    }

    // 3D Rasterizer state
    {
        D3D11_RASTERIZER_DESC rasterizerDesc;
        mem_zero(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_NONE;

        //D3D11_FILL_MODE FillMode;
        //D3D11_CULL_MODE CullMode;
        //BOOL FrontCounterClockwise;
        //INT DepthBias;
        //FLOAT DepthBiasClamp;
        //FLOAT SlopeScaledDepthBias;
        //BOOL DepthClipEnable;
        //BOOL ScissorEnable;
        //BOOL MultisampleEnable;
        //BOOL AntialiasedLineEnable;

        //D3D11_FILL_SOLID,
        //    D3D11_CULL_BACK,
        //    true,
        //    0,
        //    0.f,
        //    0.f,
        //    true,
        //    false,
        //    true,
        //    false
        device->CreateRasterizerState(&rasterizerDesc, &pSr3D);
    }

    // 3D Blend state
    {
        D3D11_BLEND_DESC blendDesc;
        mem_zero(&blendDesc, sizeof(D3D11_BLEND_DESC));
        blendDesc.RenderTarget->RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&blendDesc, &pBs3D);
    }

    // 3D Sampler state
    {
        D3D11_SAMPLER_DESC samplerDesc;
        mem_zero(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
        samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MaxAnisotropy = 4;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
        device->CreateSamplerState(&samplerDesc, &pSs3D);
    }
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

void gfx_setup3d(struct sCamera* pActiveCamera)
{
    // Set render states
    deviceContext->OMSetDepthStencilState(pDs3D, 1);
    deviceContext->RSSetState(pSr3D);
    deviceContext->OMSetBlendState(pBs3D, NULL, 0xffffffff);
    deviceContext->PSSetSamplers(0, 1, &pSs3D);

    // Bind the shaders
    deviceContext->IASetInputLayout(il3d);
    deviceContext->VSSetShader(vs3d, nullptr, 0);
    deviceContext->PSSetShader(ps3d, nullptr, 0);

    // Bind the matrices
    deviceContext->VSSetConstantBuffers(0, 1, &proj3dBuffer);
    deviceContext->VSSetConstantBuffers(1, 1, &view3dBuffer);
    deviceContext->VSSetConstantBuffers(2, 1, &world3dBuffer);

    // Update view matrix with current camera
    float dir[3];
    mat_v3sub(pActiveCamera->lookAt, pActiveCamera->pos, dir);
    mat_lookAt(pActiveCamera->transform, pActiveCamera->pos, dir, GFX_Z);
    D3D11_MAPPED_SUBRESOURCE map;
    deviceContext->Map(view3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    mem_cpy(map.pData, pActiveCamera->transform, 64);
    deviceContext->Unmap(view3dBuffer, 0);
}

void gfx_beginFrame()
{
    deviceContext->ClearRenderTargetView(renderTargetView, GFX_BLACK);
}

void gfx_drawModel(sModel& model)
{
    // Set world matrix
    D3D11_MAPPED_SUBRESOURCE map;
    deviceContext->Map(world3dBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    mem_cpy(map.pData, model.transform, 64);
    deviceContext->Unmap(world3dBuffer, 0);

    for (int i = 0; i < model.count; ++i)
    {
        sMesh* pMesh = model.meshes[i];

        deviceContext->PSSetShaderResources(0, 1, &pMesh->texture->view);
        deviceContext->PSSetShaderResources(1, 1, &pMesh->normalMap->view);
        deviceContext->IASetVertexBuffers(0, 1, &pMesh->vertexBuffer, &stride, &offset);
        deviceContext->IASetIndexBuffer(pMesh->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        deviceContext->DrawIndexed(pMesh->indexCount, 0, 0);
    }
}
