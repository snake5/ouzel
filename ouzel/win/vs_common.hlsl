
cbuffer Constants : register(b0)
{
    float4x4 modelViewProj;
}

struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct VS2PS
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

void main(in VSInput input, out VS2PS output)
{
    output.Position = mul(modelViewProj, float4(input.Position, 1));
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
}
