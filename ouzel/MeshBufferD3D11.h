// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "RendererD3D11.h"
#include "MeshBuffer.h"

namespace ouzel
{
    class MeshBufferD3D11 : public MeshBuffer
    {
    public:
        MeshBufferD3D11(Renderer* renderer);
        virtual ~MeshBufferD3D11();
        
        bool initFromData(const std::vector<uint16_t>& indices, const std::vector<Vertex>& vertices);
        
    protected:
        D3D11DynamicBuffer _vertexBuffer;
        D3D11DynamicBuffer _indexBuffer;
        UINT _indexCount;

        friend class RendererD3D11;
    };
}

