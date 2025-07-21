#include "resources/Shader/DepthFog/DepthFog.hlsli"

ConstantBuffer<DepthFogParameters> DepthFogParameter : register(b0);

Texture2D<float32_t4> gColorTexture : register(t0); // カラーテクスチャ
Texture2D<float32_t> gDepthTexture : register(t1); // 深度テクスチャ
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

// 深度値から実際のワールド距離を計算
float32_t DepthToWorldDistanceLinearized(float32_t depth)
{
    // 深度バッファは通常非線形分布
    // 簡易的な線形化: near=0.1, far=1000.0 として計算
    float32_t nearPlane = 0.1f;
    float32_t farPlane = 1000.0f;
    
    // 深度値を線形距離に変換
    float32_t linearDepth = nearPlane * farPlane / (farPlane - depth * (farPlane - nearPlane));
    return linearDepth;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    //1. カラーテクスチャから元の色を取得
    float32_t4 baseColor = gColorTexture.Sample(gSampler, input.texcoord);
    
    //2. 深度テクスチャから深度値を取得
    float32_t depth = gDepthTexture.Sample(gSampler, input.texcoord).r;
    
    //3. 深度値を世界距離に変換
    float32_t worldDistance = DepthToWorldDistanceLinearized(depth);
    
    //4. フォグファクターを計算
    float32_t fogFactor = CalculateLinearFog(worldDistance, DepthFogParameter.fogNear, DepthFogParameter.fogFar);
    
    //5. フォグを適用した最終色を計算
    float32_t3 finalColor = lerp(
        baseColor.rgb, // 元の色
        DepthFogParameter.fogColor.rgb, // フォグの色
        fogFactor * DepthFogParameter.fogDensity // フォグファクター × 密度
    );
    
    // **6. 最終結果を出力**
    output.color = float32_t4(finalColor, baseColor.a);
    
    return output;
}