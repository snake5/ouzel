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
        
        virtual bool setPixelShaderConstant(uint32_t index, const Vector3* vectors, uint32_t count) override { return setConstant(_psConstants, index, vectors, count); }
        virtual bool setPixelShaderConstant(uint32_t index, const Vector4* vectors, uint32_t count) override { return setConstant(_psConstants, index, vectors, count); }
        virtual bool setPixelShaderConstant(uint32_t index, const Matrix4* matrixes, uint32_t count) override { return setConstant(_psConstants, index, matrixes, count); }
        
        virtual bool setVertexShaderConstant(uint32_t index, const Vector3* vectors, uint32_t count) override { return setConstant(_vsConstants, index, vectors, count); }
        virtual bool setVertexShaderConstant(uint32_t index, const Vector4* vectors, uint32_t count) override { return setConstant(_vsConstants, index, vectors, count); }
        virtual bool setVertexShaderConstant(uint32_t index, const Matrix4* matrixes, uint32_t count) override { return setConstant(_vsConstants, index, matrixes, count); }
        
        template<class T> bool setConstant(std::vector<uint8_t>& buf, uint32_t index, const T* data, uint32_t count)
        {
            if (count)
            {
                size_t end = index + sizeof(*data) * count;
                buf.resize(end);
                memcpy(&buf[index], data, sizeof(*data) * count);
                return true;
            }
            return false;
        }
        
    protected:
        ID3D11VertexShader* _vertexShader;
        ID3D11PixelShader* _pixelShader;
        ID3D11InputLayout* _inputLayout;
        std::vector<uint8_t> _vsConstants;
        std::vector<uint8_t> _psConstants;

        friend class RendererD3D11;
    };
}
