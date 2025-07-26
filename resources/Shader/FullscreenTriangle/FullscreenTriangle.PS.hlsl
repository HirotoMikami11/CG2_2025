#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // テクスチャからカラーをサンプリングして出力
    output.color = gTexture.Sample(gSampler, input.texcoord);

    output.color.a = 1.0f; // オフスクリーンは常に不透明
    return output;
}