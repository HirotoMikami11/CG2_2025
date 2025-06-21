#include "resources/Shader/Sprite/Sprite.hlsli"
struct Material
{
    float32_t4 color; //色
    float32_t4x4 uvTransform; //uvTransform
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0); //SRVのregisterはt

SamplerState gSampler : register(s0); //Samplerはs

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{

    PixelShaderOutput output;

    //UV座標を変換する
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy); //同時座標系に変換してサンプリング
    
    output.color = gMaterial.color * textureColor;

    output.color.a = 1.0f;
    
    return output;
}