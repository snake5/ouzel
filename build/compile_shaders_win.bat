fxc /Zi /E"main" /Od /Fh"ps_texture.csoinl" /Vn"D3D11_SHADER_PS_TEXTURE" /T ps_4_0_level_9_1 /nologo ../ouzel/win/ps_texture.hlsl
fxc /Zi /E"main" /Od /Fh"vs_common.csoinl" /Vn"D3D11_SHADER_VS_COMMON" /T vs_4_0_level_9_1 /nologo ../ouzel/win/vs_common.hlsl
echo. > D3D11Shaders.inl
type ps_texture.csoinl >> D3D11Shaders.inl
type vs_common.csoinl >> D3D11Shaders.inl
