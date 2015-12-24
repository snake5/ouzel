// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#include <windowsx.h>
#include <stdarg.h>

#include "RendererD3D11.h"
#include "ShaderD3D11.h"
#include "TextureD3D11.h"
#include "MeshBufferD3D11.h"
#include "EventHander.h"
#include "Engine.h"
#include "Scene.h"
#include "Camera.h"
#include "Utils.h"

using namespace ouzel;


#define D3D11_DEBUG 1


#ifdef _MSC_VER
#  define VSPRINTF_LEN( str, args ) _vscprintf( str, args )
#else
#  define VSPRINTF_LEN( str, args ) vsnprintf( NULL, 0, str, args )
#endif

void D3D11FatalError(const char* err, ...)
{
    va_list args;
    va_start(args, err);
    int length = VSPRINTF_LEN(err, args);
    va_end(args);

    std::string text(length, 0);
    va_start(args, err);
    vsprintf(&text[0], err, args);
    va_end(args);

    log("%s", text.c_str());
    MessageBoxA(nullptr, text.c_str(), "Fatal Ouzel/Direct3D11 error", MB_ICONERROR);
    FatalExit(1);
}


static RendererD3D11* getRenderer(HWND window)
{
    return (RendererD3D11*) GetWindowLongPtrW(window, GWLP_USERDATA);
}

KeyboardKey winKeyToEngineCode(WPARAM wParam, LPARAM lParam)
{
    switch(wParam)
    {
    case VK_CANCEL: return KeyboardKey::CANCEL;
    case VK_BACK: return KeyboardKey::BACKSPACE;
    case VK_TAB: return KeyboardKey::TAB;
    case VK_CLEAR: return KeyboardKey::CLEAR;
    case VK_RETURN: return KeyboardKey::RETURN;
    case VK_SHIFT: return KeyboardKey::SHIFT;
    case VK_CONTROL: return KeyboardKey::CONTROL;
    case VK_MENU: return KeyboardKey::MENU;
    case VK_PAUSE: return KeyboardKey::PAUSE;
    case VK_CAPITAL: return KeyboardKey::CAPITAL;
    
    // ... Japanese ...
    
    case VK_ESCAPE: return KeyboardKey::ESCAPE;
    
    // ... IME ...
    
    case VK_SPACE: return KeyboardKey::SPACE;
    case VK_PRIOR: return KeyboardKey::PRIOR;
    case VK_NEXT: return KeyboardKey::NEXT;
    case VK_END: return KeyboardKey::END;
    case VK_HOME: return KeyboardKey::HOME;
    case VK_LEFT: return KeyboardKey::LEFT;
    case VK_UP: return KeyboardKey::UP;
    case VK_RIGHT: return KeyboardKey::RIGHT;
    case VK_DOWN: return KeyboardKey::DOWN;
    
    case VK_SELECT: return KeyboardKey::SELECT;
    case VK_PRINT: return KeyboardKey::PRINT;
    case VK_EXECUTE: return KeyboardKey::EXECUT;
    
    case VK_SNAPSHOT: return KeyboardKey::SNAPSHOT;
    case VK_INSERT: return KeyboardKey::INSERT;
    case VK_DELETE: return KeyboardKey::DELETE;
    case VK_HELP: return KeyboardKey::HELP;
    
    case '0': return KeyboardKey::KEY_0;
    case '1': return KeyboardKey::KEY_1;
    case '2': return KeyboardKey::KEY_2;
    case '3': return KeyboardKey::KEY_3;
    case '4': return KeyboardKey::KEY_4;
    case '5': return KeyboardKey::KEY_5;
    case '6': return KeyboardKey::KEY_6;
    case '7': return KeyboardKey::KEY_7;
    case '8': return KeyboardKey::KEY_8;
    case '9': return KeyboardKey::KEY_9;
    
    case 'A': return KeyboardKey::KEY_A;
    case 'B': return KeyboardKey::KEY_B;
    case 'C': return KeyboardKey::KEY_C;
    case 'D': return KeyboardKey::KEY_D;
    case 'E': return KeyboardKey::KEY_E;
    case 'F': return KeyboardKey::KEY_F;
    case 'G': return KeyboardKey::KEY_G;
    case 'H': return KeyboardKey::KEY_H;
    case 'I': return KeyboardKey::KEY_I;
    case 'J': return KeyboardKey::KEY_J;
    case 'K': return KeyboardKey::KEY_K;
    case 'L': return KeyboardKey::KEY_L;
    case 'M': return KeyboardKey::KEY_M;
    case 'N': return KeyboardKey::KEY_N;
    case 'O': return KeyboardKey::KEY_O;
    case 'P': return KeyboardKey::KEY_P;
    case 'Q': return KeyboardKey::KEY_Q;
    case 'R': return KeyboardKey::KEY_R;
    case 'S': return KeyboardKey::KEY_S;
    case 'T': return KeyboardKey::KEY_T;
    case 'U': return KeyboardKey::KEY_U;
    case 'V': return KeyboardKey::KEY_V;
    case 'W': return KeyboardKey::KEY_W;
    case 'X': return KeyboardKey::KEY_X;
    case 'Y': return KeyboardKey::KEY_Y;
    case 'Z': return KeyboardKey::KEY_Z;
    
    case VK_LWIN: return KeyboardKey::LWIN;
    case VK_RWIN: return KeyboardKey::RWIN;
    case VK_APPS: return KeyboardKey::MENU;
    case VK_SLEEP: return KeyboardKey::SLEEP;
    
    case VK_NUMPAD0: return KeyboardKey::NUMPAD0;
    case VK_NUMPAD1: return KeyboardKey::NUMPAD1;
    case VK_NUMPAD2: return KeyboardKey::NUMPAD2;
    case VK_NUMPAD3: return KeyboardKey::NUMPAD3;
    case VK_NUMPAD4: return KeyboardKey::NUMPAD4;
    case VK_NUMPAD5: return KeyboardKey::NUMPAD5;
    case VK_NUMPAD6: return KeyboardKey::NUMPAD6;
    case VK_NUMPAD7: return KeyboardKey::NUMPAD7;
    case VK_NUMPAD8: return KeyboardKey::NUMPAD8;
    case VK_NUMPAD9: return KeyboardKey::NUMPAD9;
    
    case VK_MULTIPLY: return KeyboardKey::MULTIPLY;
    case VK_ADD: return KeyboardKey::ADD;
    case VK_SEPARATOR: return KeyboardKey::SEPARATOR;
    case VK_SUBTRACT: return KeyboardKey::SUBTRACT;
    case VK_DECIMAL: return KeyboardKey::DECIMAL;
    case VK_DIVIDE: return KeyboardKey::DIVIDE;
    
    case VK_F1: return KeyboardKey::F1;
    case VK_F2: return KeyboardKey::F2;
    case VK_F3: return KeyboardKey::F3;
    case VK_F4: return KeyboardKey::F4;
    case VK_F5: return KeyboardKey::F5;
    case VK_F6: return KeyboardKey::F6;
    case VK_F7: return KeyboardKey::F7;
    case VK_F8: return KeyboardKey::F8;
    case VK_F9: return KeyboardKey::F9;
    case VK_F10: return KeyboardKey::F10;
    case VK_F11: return KeyboardKey::F11;
    case VK_F12: return KeyboardKey::F12;
    case VK_F13: return KeyboardKey::F13;
    case VK_F14: return KeyboardKey::F14;
    case VK_F15: return KeyboardKey::F15;
    case VK_F16: return KeyboardKey::F16;
    case VK_F17: return KeyboardKey::F17;
    case VK_F18: return KeyboardKey::F18;
    case VK_F19: return KeyboardKey::F19;
    case VK_F20: return KeyboardKey::F20;
    case VK_F21: return KeyboardKey::F21;
    case VK_F22: return KeyboardKey::F22;
    case VK_F23: return KeyboardKey::F23;
    case VK_F24: return KeyboardKey::F24;
    
    case VK_NUMLOCK: return KeyboardKey::NUMLOCK;
    case VK_SCROLL: return KeyboardKey::SCROLL;
    case VK_LSHIFT: return KeyboardKey::LSHIFT;
    case VK_RSHIFT: return KeyboardKey::RSHIFT;
    case VK_LCONTROL: return KeyboardKey::LCONTROL;
    case VK_RCONTROL: return KeyboardKey::RCONTROL;
    case VK_LMENU: return KeyboardKey::LMENU;
    case VK_RMENU: return KeyboardKey::RMENU;
    
    case VK_OEM_1: return KeyboardKey::OEM_1;
    case VK_OEM_PLUS: return KeyboardKey::PLUS;
    case VK_OEM_COMMA: return KeyboardKey::COMMA;
    case VK_OEM_MINUS: return KeyboardKey::MINUS;
    case VK_OEM_PERIOD: return KeyboardKey::PERIOD;
    case VK_OEM_2: return KeyboardKey::OEM_2;
    case VK_OEM_3: return KeyboardKey::OEM_3;
    case VK_OEM_4: return KeyboardKey::OEM_4;
    case VK_OEM_5: return KeyboardKey::OEM_5;
    case VK_OEM_6: return KeyboardKey::OEM_6;
    case VK_OEM_7: return KeyboardKey::OEM_7;
    case VK_OEM_8: return KeyboardKey::OEM_8;
    case VK_OEM_AX: return KeyboardKey::OEM_AX;
    case VK_OEM_102: return KeyboardKey::OEM_102;
    
    // ... misc keys ...
    }
    return KeyboardKey::NONE;
}

static void keyEvent(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    bool isDown = msg == WM_KEYDOWN;
    KeyboardKey key = winKeyToEngineCode(wParam, lParam);
    RendererD3D11* renderer = getRenderer(window);

    Event event;
    memset(&event, 0, sizeof(event));
    event.type = isDown ? Event::Type::KEY_DOWN : Event::Type::KEY_UP;
    event.keyboardEvent.key = key;
    if (wParam & MK_SHIFT)
    {
        event.keyboardEvent.shiftDown = true;
    }
    if (wParam & MK_CONTROL)
    {
        event.keyboardEvent.controlDown = true;
    }
    renderer->getEngine()->handleEvent(event);
}

static void mouseButtonEvent(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    bool isDown = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
    MouseButton button = MouseButton::LBUTTON;
    if(msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
    {
        button = MouseButton::RBUTTON;
    }
    else if(msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
    {
        button = MouseButton::MBUTTON;
    }
    RendererD3D11* renderer = getRenderer(window);
    Vector2 pos((float) GET_X_LPARAM(lParam), (float) GET_Y_LPARAM(lParam));
    
    Event event;
    memset(&event, 0, sizeof(event));
    event.type = isDown ? Event::Type::MOUSE_DOWN : Event::Type::MOUSE_UP;
    event.mouseEvent.button = button;
    event.mouseEvent.position = renderer->absoluteToWorldLocation(pos);
    renderer->getEngine()->handleEvent(event);
}

static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_KEYUP:
    case WM_KEYDOWN:
        keyEvent(window, msg, wParam, lParam);
        break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        mouseButtonEvent(window, msg, wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, msg, wParam, lParam);
}


static int CreateRasterizerState(ID3D11Device* device, D3D11_RASTERIZER_DESC* desc, ID3D11RasterizerState** out)
{
    HRESULT hr = device->CreateRasterizerState(desc, out);
    if(FAILED(hr) || !*out)
    {
        D3D11FatalError("Failed to create D3D11 rasterizer state");
        return -1;
    }
    return 0;
}

static int CreateBlendState(ID3D11Device* device, D3D11_BLEND_DESC* desc, ID3D11BlendState** out)
{
    HRESULT hr = device->CreateBlendState(desc, out);
    if(FAILED(hr) || !*out)
    {
        D3D11FatalError("Failed to create D3D11 blend state");
        return -1;
    }
    return 0;
}

static int CreateSamplerState(ID3D11Device* device, D3D11_SAMPLER_DESC* desc, ID3D11SamplerState** out)
{
    HRESULT hr = device->CreateSamplerState(desc, out);
    if(FAILED(hr) || !*out)
    {
        D3D11FatalError("Failed to create D3D11 sampler state");
        return -1;
    }
    return 0;
}

static int BufferUpload(ID3D11DeviceContext* ctx, ID3D11Buffer* buf,
    bool isDynamic, bool discard, const void* data, UINT size)
{
    if(isDynamic)
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubRsrc;
        HRESULT hr = ctx->Map(buf, 0, discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE, 0, &mappedSubRsrc);
        if(FAILED(hr))
        {
            D3D11FatalError("Failed to lock D3D11 buffer");
            return -1;
        }
        
        if(data)
            memcpy(mappedSubRsrc.pData, data, size);
        else
            memset(mappedSubRsrc.pData, 0, size);
        
        ctx->Unmap(buf, 0);
    }
    else
    {
        ctx->UpdateSubresource(buf, 0, NULL, data, size, 0);
    }
    return 0;
}

static int BufferCreate(ID3D11DeviceContext* ctx, ID3D11Device* device,
    UINT numbytes, bool isDynamic, D3D11_BIND_FLAG bindtype, const void* data, ID3D11Buffer** out)
{
    const char* what = "?";
    if(bindtype == D3D11_BIND_VERTEX_BUFFER) what = "vertex";
    if(bindtype == D3D11_BIND_INDEX_BUFFER) what = "index";
    if(bindtype == D3D11_BIND_CONSTANT_BUFFER) what = "constant";
    
    D3D11_BUFFER_DESC bufferDesc;
    memset(&bufferDesc, 0, sizeof(bufferDesc));
    
    bufferDesc.ByteWidth = numbytes;
    bufferDesc.Usage = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = bindtype;
    bufferDesc.CPUAccessFlags = isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    
    D3D11_SUBRESOURCE_DATA subResourceData;
    memset(&subResourceData, 0, sizeof(subResourceData));
    subResourceData.pSysMem = data;
    
    HRESULT hr = device->CreateBuffer(&bufferDesc, data ? &subResourceData : NULL, out);
    if(FAILED(hr) || *out == NULL)
    {
        D3D11FatalError("Failed to create D3D11 %s buffer (size=%u, dynamic=%s)", what, numbytes, isDynamic ? "true" : "false");
        return -1;
    }
    
    return isDynamic && data == nullptr ? BufferUpload(ctx, *out, true, true, NULL, numbytes) : 0;
}


D3D11DynamicBuffer::D3D11DynamicBuffer(D3D11_BIND_FLAG bindtype) : _buffer(NULL), _currSize(0), _bindType(bindtype)
{
}

D3D11DynamicBuffer::~D3D11DynamicBuffer()
{
    if(_buffer)
    {
        D3D11FatalError("UNFREED DYNAMIC BUFFER");
    }
}

void D3D11DynamicBuffer::Free()
{
    SAFE_RELEASE(_buffer);
}

void D3D11DynamicBuffer::Upload(ID3D11Device* dev, ID3D11DeviceContext* ctx, const void* data, UINT size)
{
    if(!size)
        return;
    if(size > _currSize)
    {
        SAFE_RELEASE(_buffer);
        BufferCreate(ctx, dev, size, true, _bindType, data, &_buffer);
        _currSize = size;
    }
    else
    {
        BufferUpload(ctx, _buffer, true, true, data, size);
    }
}
void D3D11DynamicBuffer::DynSetZero(ID3D11Device* dev, ID3D11DeviceContext* ctx, UINT size)
{
    Upload(dev, ctx, NULL, size);
}


RendererD3D11::RendererD3D11(const Size2& size, bool fullscreen, Engine* engine):
    Renderer(size, fullscreen, engine, Driver::DIRECT3D11),
    _fullscreen(fullscreen),
    _vertexBuffer(D3D11_BIND_VERTEX_BUFFER),
    _constantBuffer(D3D11_BIND_CONSTANT_BUFFER)
{
    initWindow();
    initD3D11();
    recalculateProjection();
}

void RendererD3D11::initWindow()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"OuzelWindow";
    RegisterClassExW(&wc);
    
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    if (_fullscreen)
    {
        style = WS_POPUP;
        x = 0;
        y = 0;
    }
    RECT windowRect = {0, 0, (int) _size.width, (int) _size.height};
    AdjustWindowRect(&windowRect, style, FALSE);
    
    _window = CreateWindowExW(
        NULL,
        L"OuzelWindow",
        L"Ouzel",
        style,
        x,
        y,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hInstance,
        NULL);
    
    SetWindowLongPtrW(_window, GWLP_USERDATA, (LONG_PTR)this);
    ShowWindow(_window, SW_SHOW);
}

void RendererD3D11::initD3D11()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    memset(&swapChainDesc, 0, sizeof(swapChainDesc));
    
    swapChainDesc.BufferDesc.Width = (int) _size.width;
    swapChainDesc.BufferDesc.Height = (int) _size.height;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = _fullscreen ? 60 : 0; // TODO refresh rate?
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    swapChainDesc.SampleDesc.Count = 1; // TODO MSAA?
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = _window;
    swapChainDesc.Windowed = _fullscreen == false;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    UINT deviceCreationFlags = 0;
#if D3D11_DEBUG
    deviceCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, // adapter
        D3D_DRIVER_TYPE_HARDWARE,
        NULL, // software rasterizer (unused)
        deviceCreationFlags,
        NULL, // feature levels
        0, // ^^
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &_swapChain,
        &_device,
        NULL,
        &_context
    );
    if(FAILED(hr))
    {
        D3D11FatalError("Failed to create the D3D11 device");
        return;
    }
    
    // Backbuffer
    hr = _swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**) &_backBuffer);
    if(FAILED(hr) || _backBuffer == NULL)
    {
        D3D11FatalError("Failed to retrieve D3D11 backbuffer");
        return;
    }
    
    hr = _device->CreateRenderTargetView(_backBuffer, NULL, &_rtView);
    if(FAILED(hr) || _rtView == NULL)
    {
        D3D11FatalError("Failed to create D3D11 render target view");
        return;
    }
    
    // Sampler state
    D3D11_SAMPLER_DESC samplerStateDesc =
    {
        D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_WRAP,
        D3D11_TEXTURE_ADDRESS_WRAP,
        0, 1, D3D11_COMPARISON_NEVER, {0,0,0,0}, 0, D3D11_FLOAT32_MAX
    };
    if(CreateSamplerState(_device, &samplerStateDesc, &_samplerState))
    {
        return;
    }
    
    // Rasterizer state
    D3D11_RASTERIZER_DESC rasterStateDesc =
    {
        D3D11_FILL_SOLID,
        D3D11_CULL_NONE,
        FALSE, // front = ccw?
        0, 0, 0, // depth bias, clamp, slope scale
        FALSE, // depth clip
        FALSE, // scissor test
        FALSE, // TODO MSAA enable?
        TRUE, // AA lines
    };
    if(CreateRasterizerState(_device, &rasterStateDesc, &_rasterizerState))
    {
        return;
    }
    
    // Blending state
#if 1
    D3D11_BLEND_DESC blendStateDesc = { FALSE, FALSE }; // alpha to coverage, independent blend
    D3D11_RENDER_TARGET_BLEND_DESC targetBlendDesc =
    {
        TRUE, // enable blending
        D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, // color blend source/dest factors, op
        D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, // alpha blend source/dest factors, op
        D3D11_COLOR_WRITE_ENABLE_ALL, // color write mask
    };
    blendStateDesc.RenderTarget[0] = targetBlendDesc;
    if(CreateBlendState(_device, &blendStateDesc, &_blendState))
    {
        return;
    }
#endif
    
    // Depth/stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc =
    {
        FALSE, // enable depth?
        // ...
    };
    hr = _device->CreateDepthStencilState(&depthStencilStateDesc, &_depthStencilState);
    if(FAILED(hr) || !_depthStencilState)
    {
        D3D11FatalError("Failed to create D3D11 depth stencil state");
        return;
    }
    
    Shader* textureShader = loadShaderFromFiles("ps_texture.cso", "vs_common.cso");
    assert(textureShader && "Failed to load texture shader");
    _shaders[SHADER_TEXTURE] = textureShader;

    D3D11_VIEWPORT viewport = { 0, 0, _size.width, _size.height, 0.0f, 1.0f };
    _context->RSSetViewports(1, &viewport);
    _context->OMSetRenderTargets(1, &_rtView, nullptr);
}

RendererD3D11::~RendererD3D11()
{
    freeInternalResources();

    SAFE_RELEASE(_depthStencilState);
    SAFE_RELEASE(_blendState);
    SAFE_RELEASE(_rasterizerState);
    SAFE_RELEASE(_samplerState);
    _vertexBuffer.Free();
    _constantBuffer.Free();
    SAFE_RELEASE(_rtView);
    SAFE_RELEASE(_backBuffer);
    SAFE_RELEASE(_swapChain);
    
#if D3D11_DEBUG
    ID3D11Debug *d3dDebug = nullptr;
    if (SUCCEEDED(_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
    {
        _context->ClearState();
        _context->Flush();
        SAFE_RELEASE(_context);
        d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
        d3dDebug->Release();
    }
#endif
    SAFE_RELEASE(_context);
    SAFE_RELEASE(_device);
    
    DestroyWindow(_window);
}

void RendererD3D11::clear()
{
    float color[4] = { _clearColor.getR(), _clearColor.getG(), _clearColor.getB(), _clearColor.getA() };
    _context->ClearRenderTargetView(_rtView, color);
}

void RendererD3D11::flush()
{
    _swapChain->Present(1 /* TODO vsync off? */, 0);
}

Shader* RendererD3D11::loadShaderFromFiles(const std::string& fragmentShader, const std::string& vertexShader)
{
    ShaderD3D11* shader = new ShaderD3D11(this);
    
    if (!shader->initFromFiles(fragmentShader, vertexShader))
    {
        delete shader;
        shader = nullptr;
    }
    
    return shader;
}

Texture* RendererD3D11::loadTextureFromFile(const std::string& filename)
{
    TextureD3D11* texture = new TextureD3D11(this);
    
    if (!texture->initFromFile(filename))
    {
        delete texture;
        texture = nullptr;
    }
    
    return texture;
}

MeshBuffer* RendererD3D11::createMeshBuffer(const std::vector<uint16_t>& indices, const std::vector<Vertex>& vertices)
{
    MeshBufferD3D11* meshBuffer = new MeshBufferD3D11(this);
    
    if (!meshBuffer->initFromData(indices, vertices))
    {
        delete meshBuffer;
        meshBuffer = nullptr;
    }
    
    return meshBuffer;
}

bool RendererD3D11::drawMeshBuffer(MeshBuffer* meshBuffer, const Matrix4& transform)
{
    auto buffer = (MeshBufferD3D11*) meshBuffer;
    auto shader = (ShaderD3D11*) _activeShader;

    Matrix4 finalTransform = _projection * _engine->getScene()->getCamera()->getTransform() * transform;
    _constantBuffer.Upload(_device, _context, &finalTransform, sizeof(Matrix4));
    
    ID3D11Buffer* constantBuffers[1] = { _constantBuffer };
    _context->VSSetConstantBuffers(0, 1, constantBuffers);
    _context->VSSetShader(shader->_vertexShader, NULL, 0);
    _context->PSSetShader(shader->_pixelShader, NULL, 0);
    _context->RSSetState(_rasterizerState);
    _context->OMSetBlendState(_blendState, NULL, 0xffffffff);
    _context->OMSetDepthStencilState(_depthStencilState, 0);
    
    ID3D11ShaderResourceView* rsrcViews[TEXTURE_LAYERS];
    ID3D11SamplerState* samplerStates[TEXTURE_LAYERS];
    for (int i = 0; i < TEXTURE_LAYERS; ++i)
    {
        auto texture = (TextureD3D11*) _activeTextures[i];
        rsrcViews[i] = texture ? texture->_rsrcView : nullptr;
        samplerStates[i] = texture ? _samplerState : nullptr;
    }
    _context->PSSetShaderResources(0, TEXTURE_LAYERS, rsrcViews);
    _context->PSSetSamplers(0, TEXTURE_LAYERS, samplerStates);
    
    _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _context->IASetInputLayout(shader->_inputLayout);
    ID3D11Buffer* buffers[1] = { buffer->_vertexBuffer };
    UINT strides[1] = { sizeof(Vertex) };
    UINT offsets[1] = { 0 };
    _context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
    _context->IASetIndexBuffer(buffer->_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    _context->DrawIndexed(buffer->_indexCount, 0, 0);
    
    return true;
}

