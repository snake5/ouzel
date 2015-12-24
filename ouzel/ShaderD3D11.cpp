// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <fstream>
#include "ShaderD3D11.h"
#include "Engine.h"
#include "FileSystem.h"
#include "Utils.h"


using namespace ouzel;


typedef std::vector<uint8_t> ByteArray;

bool LoadBinaryFile(FileSystem* fs, const std::string& name, ByteArray& out)
{
    std::string path = fs->getPath(name);
    std::ifstream file(path, std::ios::binary);
    if (file.fail())
    {
        log("failed to open file '%s' (real path = %s)", name.c_str(), path.c_str());
        return false;
    }
    file.seekg(0, std::ios::end);
    out.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    out.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if(file.fail())
    {
        log("failed to load file '%s' (real path = %s)", name.c_str(), path.c_str());
    }
    return file.fail() == false;
}

template<class T, size_t N> size_t CountOf(const T(&)[N])
{
    return N;
}


const D3D11_INPUT_ELEMENT_DESC g_VertexInputElements[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};


ShaderD3D11::ShaderD3D11(Renderer* renderer):
    Shader(renderer), _vertexShader(nullptr), _pixelShader(nullptr), _inputLayout(nullptr)
{
}

ShaderD3D11::~ShaderD3D11()
{
    SAFE_RELEASE(_vertexShader);
    SAFE_RELEASE(_pixelShader);
    SAFE_RELEASE(_inputLayout);
}

bool ShaderD3D11::initFromFiles(const std::string& fragmentShader, const std::string& vertexShader)
{
    if (!Shader::initFromFiles(fragmentShader, vertexShader))
    {
        return false;
    }
    
    auto fs = _renderer->getEngine()->getFileSystem();
    ByteArray vsData, psData;
    if(LoadBinaryFile(fs, vertexShader, vsData) == false || LoadBinaryFile(fs, fragmentShader, psData) == false)
    {
        return false;
    }
    
    initFromBytecode(psData.data(), psData.size(), vsData.data(), vsData.size(), fragmentShader, vertexShader);
    return true;
}

bool ShaderD3D11::initFromStrings(const std::string& fragmentShader, const std::string& vertexShader)
{
    if (!Shader::initFromStrings(fragmentShader, vertexShader))
    {
        return false;
    }
    
    std::string file = "<internal>";
    initFromBytecode(fragmentShader.data(), fragmentShader.size(), vertexShader.data(), vertexShader.size(), file, file);
    return true;
}

void ShaderD3D11::initFromBytecode(const void* psCode, size_t psSize, const void* vsCode, size_t vsSize, const std::string& psFile, const std::string& vsFile)
{
    auto renderer = (RendererD3D11*) _renderer;
    
    HRESULT hr = renderer->_device->CreateVertexShader(vsCode, vsSize, NULL, &_vertexShader);
    if(FAILED(hr) || !_vertexShader)
    {
        D3D11FatalError("Failed to create a D3D11 vertex shader from file '%s'", vsFile.c_str());
    }
    
    hr = renderer->_device->CreatePixelShader(psCode, psSize, NULL, &_pixelShader);
    if(FAILED(hr) || !_pixelShader)
    {
        D3D11FatalError("Failed to create a D3D11 pixel shader from file '%s'", psFile.c_str());
    }
    
    hr = renderer->_device->CreateInputLayout(
        g_VertexInputElements,
        (UINT) CountOf(g_VertexInputElements),
        vsCode,
        vsSize,
        &_inputLayout);
    if(FAILED(hr) || !_inputLayout)
    {
        D3D11FatalError("Failed to create D3D11 input layout for vertex shader '%s'", vsFile.c_str());
    }
}

uint32_t ShaderD3D11::getPixelShaderConstantId(const std::string& name)
{
    return 0;
}

uint32_t ShaderD3D11::getVertexShaderConstantId(const std::string& name)
{
    return 0;
}

