#include "resources/Shader/Vignette/Vignette.hlsli"

ConstantBuffer<VignetteParameters> VignetteParameter : register(b0);

Texture2D<float32_t4> gTexture : register(t0); // カラーテクスチャのみ
SamplerState gSampler : register(s0);

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // 1. テクスチャから元の色を取得
    float32_t4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 2. ビネット効果を適用
    float32_t3 processedColor = ApplyVignette(
        originalColor.rgb,
        input.texcoord,
        VignetteParameter
    );
    
    // 3. 最終結果を出力（アルファチャンネルは保持）
    output.color = float32_t4(processedColor, originalColor.a);
    
    return output;
}