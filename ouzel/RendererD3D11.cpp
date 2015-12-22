// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#include <stdarg.h>

#include "RendererD3D11.h"
#include "Utils.h"

using namespace ouzel;

HWND GMainWindow;


#define SAFE_RELEASE(x) if(x){ (x)->Release(); x = NULL; }


#ifdef _MSC_VER
#  define VSPRINTF_LEN( str, args ) _vscprintf( str, args )
#else
#  define VSPRINTF_LEN( str, args ) vsnprintf( NULL, 0, str, args )
#endif

static void D3D11FatalError(const char* err, ...)
{
	va_list args;
	va_start(args, err);
	int length = VSPRINTF_LEN(err, args);
	va_end(args);

	std::string text(length, 0);
	va_start(args, err);
	vsprintf_s(&text[0], length, err, args);
	va_end(args);

	log("%s", text.c_str());
	MessageBoxA(nullptr, text.c_str(), "Fatal Ouzel/Direct3D11 error", MB_ICONERROR);
    FatalExit(1);
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
    _vertexBuffer(D3D11_BIND_VERTEX_BUFFER),
    _constantBuffer(D3D11_BIND_CONSTANT_BUFFER)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    memset(&swapChainDesc, 0, sizeof(swapChainDesc));
    
    swapChainDesc.BufferDesc.Width = (int) size.width;
    swapChainDesc.BufferDesc.Height = (int) size.height;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = fullscreen ? 60 : 0; // TODO refresh rate?
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    swapChainDesc.SampleDesc.Count = 1; // TODO MSAA?
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = GMainWindow;
    swapChainDesc.Windowed = fullscreen == false;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, // adapter
        D3D_DRIVER_TYPE_HARDWARE,
        NULL, // software rasterizer (unused)
        0,//D3D11_CREATE_DEVICE_DEBUG, // flags
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
