// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "RendererOGL.h"
#include "TextureOGL.h"
#include "RenderTargetOGL.h"
#include "ShaderOGL.h"
#include "MeshBufferOGL.h"
#include "Engine.h"
#include "Scene.h"
#include "Camera.h"
#include "Utils.h"
#include "ColorPSOGL.h"
#include "ColorVSOGL.h"
#include "TexturePSOGL.h"
#include "TextureVSOGL.h"

namespace ouzel
{
    RendererOGL::RendererOGL(const Size2& size, bool fullscreen, Engine* engine):
        Renderer(size, fullscreen, engine, Driver::OPENGL)
    {
        recalculateProjection();
    }
    
    bool RendererOGL::initOpenGL(uint32_t width, uint32_t height)
    {
        //glEnable(GL_DEPTH_TEST);
        glClearColor(_clearColor.getR(), _clearColor.getG(), _clearColor.getB(), _clearColor.getA());
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        //glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        
        // precomputed alpha
        //glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        //glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        
        if (checkOpenGLErrors())
        {
            return false;
        }
        
        Shader* textureShader = loadShaderFromBuffers(TEXTURE_PIXEL_SHADER_OGL, sizeof(TEXTURE_PIXEL_SHADER_OGL), TEXTURE_VERTEX_SHADER_OGL, sizeof(TEXTURE_VERTEX_SHADER_OGL));
        if (textureShader)
        {
            _shaders[SHADER_TEXTURE] = textureShader;
        }
        
        Shader* colorShader = loadShaderFromBuffers(COLOR_PIXEL_SHADER_OGL, sizeof(COLOR_PIXEL_SHADER_OGL), COLOR_VERTEX_SHADER_OGL, sizeof(COLOR_VERTEX_SHADER_OGL));
        if (colorShader)
        {
            _shaders[SHADER_COLOR] = colorShader;
        }
        
        _ready = true;
        
        _engine->begin();
        
        return true;
    }
    
    bool RendererOGL::checkOpenGLErrors()
    {
        bool error = false;
        
        while (GLenum error = glGetError() != GL_NO_ERROR)
        {
            printf("OpenGL error: ");
            
            switch (error)
            {
                case GL_INVALID_ENUM: printf("GL_INVALID_ENUM"); break;
                case GL_INVALID_VALUE: printf("GL_INVALID_VALUE"); break;
                case GL_INVALID_OPERATION: printf("GL_INVALID_OPERATION"); break;
                case GL_OUT_OF_MEMORY: printf("GL_OUT_OF_MEMORY"); break;
                case GL_INVALID_FRAMEBUFFER_OPERATION: printf("GL_INVALID_FRAMEBUFFER_OPERATION"); break;
                default: printf("Unknown error");
            }
            
            printf(" (%x)\n", error);
            
            error = true;
        }
        
        return error;
    }
    
    void RendererOGL::setClearColor(Color color)
    {
        Renderer::setClearColor(color);
        
        glClearColor(_clearColor.getR(), _clearColor.getG(), _clearColor.getB(), _clearColor.getA());
    }

    void RendererOGL::recalculateProjection()
    {
        Renderer::recalculateProjection();
        
        if (_ready)
        {
            glViewport(0, 0, _size.width, _size.height);
        }
    }
    
    void RendererOGL::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkOpenGLErrors();
    }
    
    void RendererOGL::flush()
    {
        glFlush();
        checkOpenGLErrors();
    }
    
    Texture* RendererOGL::loadTextureFromFile(const std::string& filename)
    {
        TextureOGL* texture = new TextureOGL(this);
        
        if (!texture->initFromFile(filename))
        {
            delete texture;
            texture = nullptr;
        }
        
        return texture;
    }
    
    bool RendererOGL::activateTexture(Texture* texture, uint32_t layer)
    {
        if (!Renderer::activateTexture(texture, layer))
        {
            return false;
        }
        
        if (texture)
        {
            TextureOGL* textureOGL = static_cast<TextureOGL*>(texture);
            
            glActiveTexture(GL_TEXTURE0 + layer);
            glBindTexture(GL_TEXTURE_2D, textureOGL->getTextureId());
            
            if (checkOpenGLErrors())
            {
                return false;
            }
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        return true;
    }
    
    Shader* RendererOGL::loadShaderFromFiles(const std::string& fragmentShader, const std::string& vertexShader)
    {
        ShaderOGL* shader = new ShaderOGL(this);
        
        if (!shader->initFromFiles(fragmentShader, vertexShader))
        {
            delete shader;
            shader = nullptr;
        }
        
        return shader;
    }
    
    Shader* RendererOGL::loadShaderFromBuffers(const uint8_t* fragmentShader, uint32_t fragmentShaderSize, const uint8_t* vertexShader, uint32_t vertexShaderSize)
    {
        ShaderOGL* shader = new ShaderOGL(this);
        
        if (!shader->initFromBuffers(fragmentShader, fragmentShaderSize, vertexShader, vertexShaderSize))
        {
            delete shader;
            shader = nullptr;
        }
        
        return shader;
    }
    
    bool RendererOGL::activateShader(Shader* shader)
    {
        if (!Renderer::activateShader(shader))
        {
            return false;
        }
        
        if (shader)
        {
            ShaderOGL* shaderOGL = static_cast<ShaderOGL*>(shader);
            
            glUseProgram(shaderOGL->getProgramId());
            
            if (checkOpenGLErrors())
            {
                return false;
            }
        }
        else
        {
            glUseProgram(0);
        }
        
        return true;
    }
    
    MeshBuffer* RendererOGL::createMeshBuffer(const std::vector<uint16_t>& indices, const std::vector<Vertex>& vertices)
    {
        MeshBufferOGL* meshBuffer = new MeshBufferOGL(this);
        
        if (!meshBuffer->initFromData(indices, vertices))
        {
            delete meshBuffer;
            meshBuffer = nullptr;
        }
        
        return meshBuffer;
    }
    
    bool RendererOGL::drawMeshBuffer(MeshBuffer* meshBuffer)
    {
        if (!Renderer::drawMeshBuffer(meshBuffer))
        {
            return false;
        }
        
        MeshBufferOGL* meshBufferOGL = static_cast<MeshBufferOGL*>(meshBuffer);
        
        glBindVertexArray(meshBufferOGL->getVertexArrayId());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshBufferOGL->getIndexBufferId());
        glDrawElements(GL_TRIANGLES, meshBufferOGL->getIndexCount(), GL_UNSIGNED_SHORT, nullptr);
        
        if (checkOpenGLErrors())
        {
            return false;
        }
        
        return true;
    }
    
    void RendererOGL::drawLine(const Vector2& start, const Vector2& finish, const Color& color, const Matrix4& transform)
    {
        GLuint vertexArray = 0;
        GLuint vertexBuffer = 0;
        GLuint indexBuffer = 0;
        
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);
        
        GLfloat vertices[] = {
            start.x, start.y, -10.0f, 1.0f, color.getR(), color.getG(), color.getB(), color.getA(),
            finish.x, finish.y, -10.0f, 1.0f, color.getR(), color.getG(), color.getB(), color.getA()
        };
        
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(16));
        
        GLubyte indices[] = {0, 1};
        
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        ShaderOGL* colorShader = static_cast<ShaderOGL*>(getShader(SHADER_COLOR));
        activateShader(colorShader);
        
        GLint uniProj = glGetUniformLocation(colorShader->getProgramId(), "proj");
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, _projection.m);
        
        GLint uniView = glGetUniformLocation(colorShader->getProgramId(), "view");
        
        Camera* camera = _engine->getScene()->getCamera();
        
        if (camera)
        {
            glUniformMatrix4fv(uniView, 1, GL_FALSE, camera->getTransform().m);
        }
        else
        {
            Matrix4 temp;
            glUniformMatrix4fv(uniView, 1, GL_FALSE, temp.m);
        }
        
        GLint uniModel = glGetUniformLocation(colorShader->getProgramId(), "model");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, transform.m);
        
        glBindVertexArray(vertexArray);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_LINE_STRIP, 2, GL_UNSIGNED_BYTE, nullptr);
        
        // delete buffers
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
    }
    
    void RendererOGL::drawRectangle(const Rectangle& rectangle, const Color& color, const Matrix4& transform)
    {
        GLuint vertexArray = 0;
        GLuint vertexBuffer = 0;
        GLuint indexBuffer = 0;
        
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);
        
        GLfloat vertices[] = {
            rectangle.x, rectangle.y, -10.0f, color.getR(), color.getG(), color.getB(), color.getA(),
            rectangle.x + rectangle.width, rectangle.y, -10.0f, color.getR(), color.getG(), color.getB(), color.getA(),
            rectangle.x, rectangle.y + rectangle.height, -10.0f, color.getR(), color.getG(), color.getB(), color.getA(),
            rectangle.x + rectangle.width, rectangle.y + rectangle.height, -10.0f, color.getR(), color.getG(), color.getB(), color.getA()
        };
        
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(12));
        
        GLubyte indices[] = {0, 1, 3, 2, 0};
        
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        ShaderOGL* colorShader = static_cast<ShaderOGL*>(getShader(SHADER_COLOR));
        activateShader(colorShader);
        
        Matrix4 modelViewProj = _projection * _engine->getScene()->getCamera()->getTransform() * transform;
        
        uint32_t uniModelViewProj = colorShader->getVertexShaderConstantId("modelViewProj");
        colorShader->setVertexShaderConstant(uniModelViewProj, &modelViewProj, 1);
        
        glBindVertexArray(vertexArray);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_LINE_STRIP, 5, GL_UNSIGNED_BYTE, nullptr);
        
        // delete buffers
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
    }
    
    void RendererOGL::drawQuad(const Rectangle& rectangle, const Color& color, const Matrix4& transform)
    {
        GLuint vertexArray = 0;
        GLuint vertexBuffer = 0;
        GLuint indexBuffer = 0;
        
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);
        
        GLfloat vertices[] = {
            rectangle.x, rectangle.y, -10.0f, color.getR(), color.getG(), color.getB(), color.getA(), 0.0f, 1.0f,
            rectangle.x + rectangle.width, rectangle.y, -10.0f, color.getR(), color.getG(), color.getB(), color.getA(), 1.0f, 1.0f,
            rectangle.x, rectangle.y + rectangle.height, -10.0f, color.getR(), color.getG(), color.getB(), color.getA(), 0.0f, 0.0f,
            rectangle.x + rectangle.width, rectangle.y + rectangle.height, -10.0f, color.getR(), color.getG(), color.getB(), color.getA(), 1.0f, 0.0f
        };
        
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(12));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(16));
        
        GLubyte indices[] = {0, 1, 2, 1, 3, 2};
        
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        ShaderOGL* colorShader = static_cast<ShaderOGL*>(getShader(SHADER_TEXTURE));
        activateShader(colorShader);
        
        Matrix4 modelViewProj = _projection * _engine->getScene()->getCamera()->getTransform() * transform;
        
        uint32_t uniModelViewProj = colorShader->getVertexShaderConstantId("modelViewProj");
        colorShader->setVertexShaderConstant(uniModelViewProj, &modelViewProj, 1);
        
        glBindVertexArray(vertexArray);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
        
        // delete buffers
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &indexBuffer);
    }
}
