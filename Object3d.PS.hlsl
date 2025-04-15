#include "Object3d.hlsli"
struct Material
{
    float32_t4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);      //SRVのregisterはt
SamplerState gSampler : register(s0);  //Samplerはs

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{

    PixelShaderOutput output;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    //サンプリングしたtextureの色とマテリアルん色を乗算して合成する
    output.color = gMaterial.color * textureColor;
  
    return output;
}