#include "Glitch.hlsli"

ConstantBuffer<GlitchMaterial> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float2 uv = input.texcoord;
    
    // === シンプルなRGBシフトエフェクト ===
    
    // シフト量を設定（強めに設定してエフェクトを確認しやすくする）
    float shiftAmount = gMaterial.rgbShiftStrength * 0.01f;
    
    //// 左半分: 元のテクスチャをそのまま表示（比較用）
    //if (uv.x < 0.5f)
    //{
    //    float4 originalColor = gTexture.Sample(gSampler, uv);
    //    output.color = originalColor;
    //    return output;
    //}
    
    // 右半分: RGBシフトエフェクトを適用
    // 各色チャンネルを少しずつずらしてサンプリング
    float r = gTexture.Sample(gSampler, uv + float2(shiftAmount, 0.0f)).r;
    float g = gTexture.Sample(gSampler, uv).g;
    float b = gTexture.Sample(gSampler, uv - float2(shiftAmount, 0.0f)).b;
    
    // 合成して出力
    output.color = float4(r, g, b, 1.0f);
    
    return output;
}