// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "CompileConfig.h"

#if defined(OUZEL_PLATFORM_OSX)
#include <OpenGL/gl3.h>
#elif defined(OUZEL_PLATFORM_IOS)
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#endif

#include "Shader.h"

namespace ouzel
{
    class ShaderOGL: public Shader
    {
    public:
        ShaderOGL(Renderer* renderer);
        virtual ~ShaderOGL();
        
        virtual bool initFromBuffers(const uint8_t* fragmentShader, uint32_t fragmentShaderSize, const uint8_t* vertexShader, uint32_t vertexShaderSize) override;
        
        GLuint getProgramId() const { return _programId; }
        
        virtual uint32_t getPixelShaderConstantId(const std::string& name) override;
        virtual bool setPixelShaderConstant(uint32_t index, const Vector3* vectors, uint32_t count) override;
        virtual bool setPixelShaderConstant(uint32_t index, const Vector4* vectors, uint32_t count) override;
        virtual bool setPixelShaderConstant(uint32_t index, const Matrix4* matrices, uint32_t count) override;
        
        virtual uint32_t getVertexShaderConstantId(const std::string& name) override;
        virtual bool setVertexShaderConstant(uint32_t index, const Vector3* vectors, uint32_t count) override;
        virtual bool setVertexShaderConstant(uint32_t index, const Vector4* vectors, uint32_t count) override;
        virtual bool setVertexShaderConstant(uint32_t index, const Matrix4* matrices, uint32_t count) override;
        
    protected:
        bool checkShaderError(GLuint shader);
        
        GLuint _vertexShader;
        GLuint _fragmentShader;
        GLuint _programId;
    };
}
