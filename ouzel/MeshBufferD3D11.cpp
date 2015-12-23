// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "MeshBufferD3D11.h"

using namespace ouzel;

MeshBufferD3D11::MeshBufferD3D11(Renderer* renderer):
    MeshBuffer(renderer), _vertexBuffer(D3D11_BIND_VERTEX_BUFFER), _indexBuffer(D3D11_BIND_INDEX_BUFFER)
{
}

MeshBufferD3D11::~MeshBufferD3D11()
{
	_vertexBuffer.Free();
	_indexBuffer.Free();
}

bool MeshBufferD3D11::initFromData(const std::vector<uint16_t>& indices, const std::vector<Vertex>& vertices)
{
	auto renderer = (RendererD3D11*) _renderer;
	_indexCount = (UINT) indices.size();
	_vertexBuffer.Upload(renderer->_device, renderer->_context, vertices.data(), (UINT) vertices.size() * sizeof(Vertex));
	_indexBuffer.Upload(renderer->_device, renderer->_context, indices.data(), (UINT) indices.size() * sizeof(uint16_t));
	return true;
}

