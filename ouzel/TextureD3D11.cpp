// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "TextureD3D11.h"
#include "RendererD3D11.h"
#include "Image.h"
#include "Utils.h"

namespace ouzel
{
    TextureD3D11::TextureD3D11(Renderer* renderer):
        Texture(renderer)
    {
        
    }
    
    TextureD3D11::~TextureD3D11()
    {
        
    }

    bool TextureD3D11::initFromFile(const std::string& filename)
    {
        if (!Texture::initFromFile(filename))
        {
            return false;
        }

        AutoPtr<Image> image = new Image(_renderer->getEngine());
        if (!image->loadFromFile(filename))
        {
            return false;
        }

        RendererD3D11* rendererD3D11 = static_cast<RendererD3D11*>(_renderer);
        int width = (int)image->getSize().width;
        int height = (int)image->getSize().height;

        D3D11_TEXTURE2D_DESC textureDesc;
        memset(&textureDesc, 0, sizeof(textureDesc));
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initialData = { image->getData(), (UINT)width * 4 };
        HRESULT hr = rendererD3D11->getDevice()->CreateTexture2D(&textureDesc, &initialData, &_texture);
        if (FAILED(hr) || !_texture)
        {
            log("Could not create D3D11 texture (type=2D, width=%d, height=%d, name=%s)", width, height, filename.c_str());
            return false;
        }

        hr = rendererD3D11->getDevice()->CreateShaderResourceView(_texture, NULL, &_resourceView);
        if (FAILED(hr) || !_resourceView)
        {
            log("Could not create D3D11 shader resource view (type=2D, width=%d, height=%d, name=%s)", width, height, filename.c_str());
            return false;
        }

        _size = image->getSize();

        return true;
    }
}
