#include "Fullscreen.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting; // 使わない
    int32_t useLambertianReflectance; // 使わない
    float32_t4x4 uvTransform;
};
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // UV座標を変換する
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // 単純にテクスチャの色とマテリアルの色を乗算（ライティングなし）
    output.color = gMaterial.color * textureColor;
    
    return output;
}