#include "resources/Shader/Grayscale/Grayscale.hlsli"

ConstantBuffer<GrayscaleParameters> GrayscaleParameter : register(b0);

Texture2D<float32_t4> gTexture : register(t0); // カラーテクスチャのみ
SamplerState gSampler : register(s0);

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // 1. テクスチャから元の色を取得
    float32_t4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 2. グレースケールを適用
    float32_t3 processedColor = ApplyGrayscale(
        originalColor.rgb,
        GrayscaleParameter.grayIntensity
    );
    
    // 3. 基本色を乗算（通常は白色なので変化なし）
    processedColor *= GrayscaleParameter.color.rgb;
    
    // 4. 最終結果を出力（アルファチャンネルは保持）
    output.color = float32_t4(processedColor, originalColor.a);
    
    return output;
}