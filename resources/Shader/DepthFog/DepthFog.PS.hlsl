#include "resources/Shader/DepthFog/DepthFog.hlsli"

ConstantBuffer<DepthFogParameters> DepthFogParameter : register(b0);

Texture2D<float32_t4> gColorTexture : register(t0); // カラーテクスチャ
Texture2D<float32_t> gDepthTexture : register(t1); // 深度テクスチャ
SamplerState gSampler : register(s0);

// フォグを適用する関数
float32_t3 applyDepthFog(float32_t3 baseColor, float32_t3 fogColor, float32_t fogFactor)
{
    return lerp(baseColor, fogColor, fogFactor);
}

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // 深度テクスチャから深度値を取得
    float32_t depth = gDepthTexture.Sample(gSampler, input.texcoord).r;
    
    // 深度値を世界距離に変換
    float32_t worldDistance = DepthToWorldDistance(depth);
    
    // フォグファクターを計算
    float32_t fogFactor = CalculateLinearFog(worldDistance, DepthFogParameter.fogNear, DepthFogParameter.fogFar);
    
    // ★ 複数の値を同時に可視化
    // R: 深度値 (0-1)
    // G: worldDistance / 100.0f (0-1)  
    // B: fogFactor (0-1)
    output.color = float32_t4(
        depth, // 赤：深度値
        saturate(worldDistance / 100.0f), // 緑：距離値  
        fogFactor, // 青：フォグファクター
        1.0f
    );
    return output;
}