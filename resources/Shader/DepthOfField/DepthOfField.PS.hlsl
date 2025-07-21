#include "resources/Shader/DepthOfField/DepthOfField.hlsli"

ConstantBuffer<DepthOfFieldParameters> DepthOfFieldParameter : register(b0);

Texture2D<float32_t4> gColorTexture : register(t0); // カラーテクスチャ
Texture2D<float32_t> gDepthTexture : register(t1); // 深度テクスチャ
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};


// ガウシアンブラーの簡易版
float32_t4 ApplyGaussianBlur(Texture2D<float32_t4> colorTexture, SamplerState sampler, float2 texcoord, float32_t blurRadius)
{
    float32_t4 result = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);
    float32_t totalWeight = 0.0f;
    
    // テクスチャサイズを取得（仮定）
    float32_t2 texelSize = float32_t2(1.0f / 1920.0f, 1.0f / 1080.0f);
    
    // ガウシアンカーネルの重み（5x5）
    float32_t weights[3] = { 0.2270270270f, 0.3162162162f, 0.0702702703f };
    
    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            float32_t2 offset = float32_t2(x, y) * texelSize * blurRadius;
            float32_t2 sampleCoord = texcoord + offset;
            
            // テクスチャ境界チェック
            if (sampleCoord.x >= 0.0f && sampleCoord.x <= 1.0f &&
                sampleCoord.y >= 0.0f && sampleCoord.y <= 1.0f)
            {
                float32_t4 sampleColor = colorTexture.Sample(
                sampler, sampleCoord);
                
                // ガウシアン重みを計算
                int weightIndex = max(abs(x), abs(y));
                weightIndex = min(weightIndex, 2);
                float32_t weight = weights[weightIndex];
                
                result += sampleColor * weight;
                totalWeight += weight;
            }
        }
    }
    
    // 重み付き平均を計算
    if (totalWeight > 0.0f)
    {
        result /= totalWeight;
    }
    
    return result;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // 1. カラーテクスチャから元の色を取得
    float32_t4 baseColor = gColorTexture.Sample(gSampler, input.texcoord);
    
    // 2. 深度テクスチャから深度値を取得
    float32_t depth = gDepthTexture.Sample(gSampler, input.texcoord).r;

    // 3. 深度値を世界距離に変換
    float32_t worldDistance = DepthToWorldDistanceLinearized(depth);
    
    // 4. 焦点からの距離に基づいてボケ強度を計算
    float32_t blurFactor = CalculateBlurFactor(
        worldDistance,
        DepthOfFieldParameter.focusDistance,
        DepthOfFieldParameter.focusRange
    );
    
    // 5. ボケ強度が小さい場合は元の色をそのまま使用
    if (blurFactor < 0.01f)
    {
        output.color = baseColor;
        return output;
    }
    
    // 6. ボケ強度に応じてブラー半径を計算
    float32_t blurRadius = blurFactor * DepthOfFieldParameter.blurStrength;
    
    // 7. ブラー処理を適用
    float32_t4 blurredColor = ApplyGaussianBlur(gColorTexture, gSampler, input.texcoord, blurRadius);
    
    // 8. 元の色とぼかした色を混合
    float32_t3 finalColor = lerp(
        baseColor.rgb, // 元の色
        blurredColor.rgb, // ぼかした色
        blurFactor // ボケ強度で混合
    );
    
    // 9. 最終結果を出力
    output.color = float32_t4(finalColor, baseColor.a);
    
    return output;
}