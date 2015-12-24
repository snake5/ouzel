fxc /Zi /E"main" /Od /Fo"ps_texture.cso" /T ps_4_0_level_9_1 /nologo ../ouzel/win/ps_texture.hlsl
fxc /Zi /E"main" /Od /Fo"vs_common.cso" /T vs_4_0_level_9_1 /nologo ../ouzel/win/vs_common.hlsl
echo. > D3D11Shaders.inl
bin2h D3D11_SHADER_PS_TEXTURE < ps_texture.cso >> D3D11Shaders.inl
bin2h D3D11_SHADER_VS_COMMON < vs_common.cso >> D3D11Shaders.inl
powershell -Command "(gc D3D11Shaders.inl) -replace 'char', 'uint8_t' | Out-File D3D11Shaders.inl"
