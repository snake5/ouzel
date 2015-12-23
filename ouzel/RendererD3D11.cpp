// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#include <windowsx.h>
#include <stdarg.h>

#include "RendererD3D11.h"
#include "ShaderD3D11.h"
#include "EventHander.h"
#include "Engine.h"
#include "Utils.h"

using namespace ouzel;


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

KeyCode winKeyToEngineCode(WPARAM wParam, LPARAM lParam)
{
    switch(wParam)
    {
    case VK_PAUSE: return KeyCode::PAUSE;
    case VK_SCROLL: return KeyCode::SCROLL_LOCK;
    case VK_PRINT: return KeyCode::PRINT;
    case VK_SNAPSHOT: return KeyCode::SYSREQ;
    case VK_CANCEL: return KeyCode::BREAK;
    case VK_ESCAPE: return KeyCode::ESCAPE;
    case VK_BROWSER_BACK: return KeyCode::BACK;
    case VK_BACK: return KeyCode::BACKSPACE;
    case VK_TAB: return KeyCode::TAB;
    case VK_OEM_BACKTAB: return KeyCode::BACK_TAB;
	case VK_RETURN: return lParam & 0x1000000 ? KeyCode::KP_ENTER : KeyCode::RETURN;
    case VK_CAPITAL: return KeyCode::CAPS_LOCK;
    case VK_SHIFT: return KeyCode::SHIFT;
    case VK_LSHIFT: return KeyCode::LEFT_SHIFT;
    case VK_RSHIFT: return KeyCode::RIGHT_SHIFT;
    case VK_CONTROL: return KeyCode::CTRL;
    case VK_LCONTROL: return KeyCode::LEFT_CTRL;
    case VK_RCONTROL: return KeyCode::RIGHT_CTRL;
    case VK_MENU: return KeyCode::ALT;
    case VK_LMENU: return KeyCode::LEFT_ALT;
    case VK_RMENU: return KeyCode::RIGHT_ALT;
    case VK_APPS: return KeyCode::MENU;
    // KeyCode::HYPER ?
	case VK_INSERT: return lParam & 0x1000000 ? KeyCode::KP_INSERT : KeyCode::INSERT;
    case VK_HOME: return KeyCode::HOME;
	case VK_PRIOR: return lParam & 0x1000000 ? KeyCode::KP_PG_UP : KeyCode::PG_UP;
	case VK_DELETE: return lParam & 0x1000000 ? KeyCode::KP_DELETE : KeyCode::DELETE;
    case VK_END: return KeyCode::END;
    case VK_NEXT: return KeyCode::PG_DOWN;
    case VK_LEFT: return KeyCode::LEFT_ARROW;
    case VK_RIGHT: return KeyCode::RIGHT_ARROW;
    case VK_UP: return KeyCode::UP_ARROW;
    case VK_DOWN: return KeyCode::DOWN_ARROW;
    case VK_NUMLOCK: return KeyCode::NUM_LOCK;
    case VK_ADD: return KeyCode::KP_PLUS;
    case VK_SUBTRACT: return KeyCode::KP_MINUS;
    case VK_MULTIPLY: return KeyCode::KP_MULTIPLY;
    case VK_DIVIDE: return KeyCode::KP_DIVIDE;
    // KeyCode::KP_ENTER -- see VK_RETURN
#if 0 // should be same as the standard keys
    case : return KeyCode::KP_HOME;
    case : return KeyCode::KP_UP;
    case : return KeyCode::KP_PG_UP;
    case : return KeyCode::KP_LEFT;
    case : return KeyCode::KP_FIVE;
    case : return KeyCode::KP_RIGHT;
    case : return KeyCode::KP_END;
    case : return KeyCode::KP_DOWN;
    case : return KeyCode::KP_PG_DOWN;
    case : return KeyCode::KP_INSERT;
    case : return KeyCode::KP_DELETE;
#endif
    case VK_F1: return KeyCode::F1;
    case VK_F2: return KeyCode::F2;
    case VK_F3: return KeyCode::F3;
    case VK_F4: return KeyCode::F4;
    case VK_F5: return KeyCode::F5;
    case VK_F6: return KeyCode::F6;
    case VK_F7: return KeyCode::F7;
    case VK_F8: return KeyCode::F8;
    case VK_F9: return KeyCode::F9;
    case VK_F10: return KeyCode::F10;
    case VK_F11: return KeyCode::F11;
    case VK_F12: return KeyCode::F12;
    case VK_SPACE: return KeyCode::SPACE;
#if 0
    case : return KeyCode::EXCLAM;
    case : return KeyCode::QUOTE;
    case : return KeyCode::NUMBER;
    case : return KeyCode::DOLLAR;
    case : return KeyCode::PERCENT;
    case : return KeyCode::CIRCUMFLEX;
    case : return KeyCode::AMPERSAND;
    case : return KeyCode::APOSTROPHE;
    case : return KeyCode::LEFT_PARENTHESIS;
    case : return KeyCode::RIGHT_PARENTHESIS;
    case : return KeyCode::ASTERISK;
    case : return KeyCode::PLUS;
	case : return KeyCode::MINUS;
#endif
    case VK_OEM_COMMA: return KeyCode::COMMA;
    case VK_OEM_PERIOD: return KeyCode::PERIOD;
    // case : return KeyCode::SLASH; // which one?
    case '0': return KeyCode::KEY_0;
    case '1': return KeyCode::KEY_1;
    case '2': return KeyCode::KEY_2;
    case '3': return KeyCode::KEY_3;
    case '4': return KeyCode::KEY_4;
    case '5': return KeyCode::KEY_5;
    case '6': return KeyCode::KEY_6;
    case '7': return KeyCode::KEY_7;
    case '8': return KeyCode::KEY_8;
    case '9': return KeyCode::KEY_9;
    case VK_NUMPAD0: return KeyCode::NUMPAD_0;
    case VK_NUMPAD1: return KeyCode::NUMPAD_1;
    case VK_NUMPAD2: return KeyCode::NUMPAD_2;
    case VK_NUMPAD3: return KeyCode::NUMPAD_3;
    case VK_NUMPAD4: return KeyCode::NUMPAD_4;
    case VK_NUMPAD5: return KeyCode::NUMPAD_5;
    case VK_NUMPAD6: return KeyCode::NUMPAD_6;
    case VK_NUMPAD7: return KeyCode::NUMPAD_7;
    case VK_NUMPAD8: return KeyCode::NUMPAD_8;
    case VK_NUMPAD9: return KeyCode::NUMPAD_9;
#if 0
    case : return KeyCode::COLON;
    case : return KeyCode::SEMICOLON;
    case : return KeyCode::LESS_THAN;
    case : return KeyCode::EQUAL;
    case : return KeyCode::GREATER_THAN;
    case : return KeyCode::QUESTION;
    case : return KeyCode::AT;
    case : return KeyCode::CAPITAL_A;
    case : return KeyCode::CAPITAL_B;
    case : return KeyCode::CAPITAL_C;
    case : return KeyCode::CAPITAL_D;
    case : return KeyCode::CAPITAL_E;
    case : return KeyCode::CAPITAL_F;
    case : return KeyCode::CAPITAL_G;
    case : return KeyCode::CAPITAL_H;
    case : return KeyCode::CAPITAL_I;
    case : return KeyCode::CAPITAL_J;
    case : return KeyCode::CAPITAL_K;
    case : return KeyCode::CAPITAL_L;
    case : return KeyCode::CAPITAL_M;
    case : return KeyCode::CAPITAL_N;
    case : return KeyCode::CAPITAL_O;
    case : return KeyCode::CAPITAL_P;
    case : return KeyCode::CAPITAL_Q;
    case : return KeyCode::CAPITAL_R;
    case : return KeyCode::CAPITAL_S;
    case : return KeyCode::CAPITAL_T;
    case : return KeyCode::CAPITAL_U;
    case : return KeyCode::CAPITAL_V;
    case : return KeyCode::CAPITAL_W;
    case : return KeyCode::CAPITAL_X;
    case : return KeyCode::CAPITAL_Y;
    case : return KeyCode::CAPITAL_Z;
    case : return KeyCode::LEFT_BRACKET;
    case : return KeyCode::BACK_SLASH;
    case : return KeyCode::RIGHT_BRACKET;
    case : return KeyCode::UNDERSCORE;
    case : return KeyCode::GRAVE;
#endif
    case 'A': return KeyCode::A;
    case 'B': return KeyCode::B;
    case 'C': return KeyCode::C;
    case 'D': return KeyCode::D;
    case 'E': return KeyCode::E;
    case 'F': return KeyCode::F;
    case 'G': return KeyCode::G;
    case 'H': return KeyCode::H;
    case 'I': return KeyCode::I;
    case 'J': return KeyCode::J;
    case 'K': return KeyCode::K;
    case 'L': return KeyCode::L;
    case 'M': return KeyCode::M;
    case 'N': return KeyCode::N;
    case 'O': return KeyCode::O;
    case 'P': return KeyCode::P;
    case 'Q': return KeyCode::Q;
    case 'R': return KeyCode::R;
    case 'S': return KeyCode::S;
    case 'T': return KeyCode::T;
    case 'U': return KeyCode::U;
    case 'V': return KeyCode::V;
    case 'W': return KeyCode::W;
    case 'X': return KeyCode::X;
    case 'Y': return KeyCode::Y;
    case 'Z': return KeyCode::Z;
#if 0
    case : return KeyCode::LEFT_BRACE;
    case : return KeyCode::BAR;
    case : return KeyCode::RIGHT_BRACE;
    case : return KeyCode::TILDE;
    case : return KeyCode::EURO;
    case : return KeyCode::POUND;
    case : return KeyCode::YEN;
    case : return KeyCode::MIDDLE_DOT;
#endif
    case VK_BROWSER_SEARCH: return KeyCode::SEARCH;
#if 0
    case : return KeyCode::DPAD_LEFT;
    case : return KeyCode::DPAD_RIGHT;
    case : return KeyCode::DPAD_UP;
    case : return KeyCode::DPAD_DOWN;
    case : return KeyCode::DPAD_CENTER;
    case : return KeyCode::ENTER;
#endif
    case VK_PLAY: return KeyCode::PLAY;
    }
    return KeyCode::NONE;
}

static void keyEvent(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isDown = msg == WM_KEYDOWN;
	KeyCode key = winKeyToEngineCode(wParam, lParam);
	RendererD3D11* renderer = getRenderer(window);

	Event event;
	memset(&event, 0, sizeof(event));
	event.type = isDown ? Event::Type::KEY_DOWN : Event::Type::KEY_UP;
	event.key = (uint32_t) key;
	if (wParam & MK_SHIFT)
	{
		event.shiftDown = true;
	}
	if (wParam & MK_CONTROL)
	{
		event.controlDown = true;
	}
	renderer->getEngine()->handleEvent(event);
}

static void mouseButtonEvent(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    bool isDown = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
    int button = 0;
    if(msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
    {
        button = 1;
    }
    else if(msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
    {
        button = 2;
    }
    RendererD3D11* renderer = getRenderer(window);
    Vector2 pos((float) GET_X_LPARAM(lParam), (float) GET_Y_LPARAM(lParam));
    
    Event event;
    memset(&event, 0, sizeof(event));
    event.type = isDown ? Event::Type::MOUSE_DOWN : Event::Type::MOUSE_UP;
    event.mouseButton = button;
    event.mousePosition = renderer->absoluteToWorldLocation(pos);
    if(wParam & MK_SHIFT)
    {
        event.shiftDown = true;
    }
    if(wParam & MK_CONTROL)
    {
        event.controlDown = true;
    }
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
    
    return isDynamic ? BufferUpload(ctx, *out, true, true, NULL, numbytes) : 0;
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
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, // adapter
        D3D_DRIVER_TYPE_HARDWARE,
        NULL, // software rasterizer (unused)
        D3D11_CREATE_DEVICE_DEBUG, // flags
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
#if 0 // TODO
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
    _shaders[SHADER_TEXTURE] = textureShader;
}

RendererD3D11::~RendererD3D11()
{
    SAFE_RELEASE(_depthStencilState);
    SAFE_RELEASE(_rasterizerState);
    SAFE_RELEASE(_samplerState);
    _vertexBuffer.Free();
    _constantBuffer.Free();
    SAFE_RELEASE(_rtView);
    SAFE_RELEASE(_backBuffer);
    SAFE_RELEASE(_swapChain);
    SAFE_RELEASE(_context);
    SAFE_RELEASE(_device);
    
    DestroyWindow(_window);
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

void RendererD3D11::clear()
{
    float color[4] = { _clearColor.getR(), _clearColor.getG(), _clearColor.getB(), _clearColor.getA() };
    _context->ClearRenderTargetView(_rtView, color);
}

void RendererD3D11::flush()
{
    _swapChain->Present(1 /* TODO vsync off? */, 0);
}

bool RendererD3D11::activateTexture(Texture* texture, uint32_t layer)
{
    return true;
}
