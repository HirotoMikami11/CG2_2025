#include "Grayscale.hlsli"

ConstantBuffer<GrayscaleParameters> GrayscaleParameter : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// グレースケール変換関数（加重平均）
float3 toGrayscale(float3 color)
{
    float luminance = dot(color, float3(0.299, 0.587, 0.114));
    return float3(luminance, luminance, luminance);
}

// カラーとグレースケールをイージングする関数
//元の色とイージングのtの入れる
//0に近ければ近いほど元の色に、1に近いほどグレースケールに。
float3 applyGrayscale(float3 color, float t)
{
    float3 grayscale = toGrayscale(color);
    return lerp(color, grayscale, t);
}


struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color.rgb = applyGrayscale(output.color.rgb, 1.0f); //この1はtなのでパラメータにできる
    return output;
}