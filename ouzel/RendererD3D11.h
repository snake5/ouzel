// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <windows.h>
#include <d3d11.h>

#include "Renderer.h"


#define SAFE_RELEASE(x) if(x){ (x)->Release(); x = NULL; }

void D3D11FatalError(const char* err, ...);


namespace ouzel
{
    class D3D11DynamicBuffer
    {
    public:
        D3D11DynamicBuffer(D3D11_BIND_FLAG bindtype);
        ~D3D11DynamicBuffer();
        void Free();
        void Upload(ID3D11Device* dev, ID3D11DeviceContext* ctx, const void* data, UINT size);
        void DynSetZero(ID3D11Device* dev, ID3D11DeviceContext* ctx, UINT size);

        operator ID3D11Buffer* () const { return _buffer; }

    private:
        ID3D11Buffer* _buffer;
        UINT _currSize;
        D3D11_BIND_FLAG _bindType;

        D3D11DynamicBuffer(const D3D11DynamicBuffer&) = delete;
        D3D11DynamicBuffer& operator = (const D3D11DynamicBuffer&) = delete;
    };

    class RendererD3D11: public Renderer
    {
    public:
        RendererD3D11(const Size2& size, bool fullscreen, Engine* engine);
        virtual ~RendererD3D11();
        
        void initWindow();
        void initD3D11();
        
        virtual void clear() override;
        virtual void flush() override;
        
        virtual Shader* loadShaderFromFiles(const std::string& fragmentShader, const std::string& vertexShader) override;
        virtual Shader* loadShaderFromStrings(const std::string& fragmentShader, const std::string& vertexShader) override;
        virtual Texture* loadTextureFromFile(const std::string& filename) override;
        
        virtual MeshBuffer* createMeshBuffer(const std::vector<uint16_t>& indices, const std::vector<Vertex>& vertices);
        virtual bool drawMeshBuffer(MeshBuffer* meshBuffer, const Matrix4& transform = Matrix4());
        
    private:
        bool _fullscreen;
        HWND _window;
        
        ID3D11Device* _device;
        ID3D11DeviceContext* _context;
        IDXGISwapChain* _swapChain;
        ID3D11Texture2D* _backBuffer;
        ID3D11RenderTargetView* _rtView;
        ID3D11SamplerState* _samplerState;
        ID3D11RasterizerState* _rasterizerState;
        ID3D11BlendState* _blendState;
        ID3D11DepthStencilState* _depthStencilState;
        
        D3D11DynamicBuffer _vertexBuffer;
        D3D11DynamicBuffer _constantBuffer;
        
        friend class ShaderD3D11;
        friend class TextureD3D11;
        friend class MeshBufferD3D11;
    };
}
