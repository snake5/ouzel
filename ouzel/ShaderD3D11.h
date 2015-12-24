// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "RendererD3D11.h"
#include "Shader.h"

namespace ouzel
{
    class ShaderD3D11: public Shader
    {
    public:
        ShaderD3D11(Renderer* renderer);
        virtual ~ShaderD3D11();
        
        virtual bool initFromFiles(const std::string& fragmentShader, const std::string& vertexShader) override;
        virtual bool initFromStrings(const std::string& fragmentShader, const std::string& vertexShader) override;
        void initFromBytecode(const void* psCode, size_t psSize, const void* vsCode, size_t vsSize, const std::string& psFile, const std::string& vsFile);
        
        virtual uint32_t getPixelShaderConstantId(const std::string& name) override;
        virtual uint32_t getVertexShaderConstantId(const std::string& name) override;
        
    protected:
        ID3D11VertexShader* _vertexShader;
        ID3D11PixelShader* _pixelShader;
        ID3D11InputLayout* _inputLayout;

        friend class RendererD3D11;
    };
}
