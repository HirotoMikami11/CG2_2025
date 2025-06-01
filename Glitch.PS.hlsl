#include "Glitch.hlsli"

ConstantBuffer<GlitchMaterial> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

// ランダム関数（疑似乱数）
float random(float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

// ノイズ関数
float noise(float2 st)
{
    float2 i = floor(st);
    float2 f = frac(st);
    
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));
    
    float2 u = f * f * (3.0 - 2.0 * f);
    
    return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float2 uv = input.texcoord;
    
    // === シンプルなRGBシフトエフェクト ===
    
    // シフト量を設定（強めに設定してエフェクトを確認しやすくする）
    float shiftAmount = gMaterial.rgbShiftStrength * 0.01f;
    
    // 左半分: 元のテクスチャをそのまま表示（比較用）
    if (uv.x < 0.5f)
    {
        float4 originalColor = gTexture.Sample(gSampler, uv);
        output.color = originalColor;
        return output;
    }
    
    // 右半分: RGBシフトエフェクトを適用
    // 各色チャンネルを少しずつずらしてサンプリング
    float r = gTexture.Sample(gSampler, uv + float2(shiftAmount, 0.0f)).r;
    float g = gTexture.Sample(gSampler, uv).g;
    float b = gTexture.Sample(gSampler, uv - float2(shiftAmount, 0.0f)).b;
    
    // 合成して出力
    output.color = float4(r, g, b, 1.0f);
    
    return output;
}