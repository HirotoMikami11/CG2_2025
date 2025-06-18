#include "RGBShift.hlsli"

ConstantBuffer<RGBShiftParameters> RGBShiftParameter : register(b0);

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
    
    // RGBシフトエフェクトのみ
    float shiftAmount = RGBShiftParameter.rgbShiftStrength * 0.005f;
    
    // 時間に基づく揺らぎ
    float timeShift = sin(RGBShiftParameter.time * 10.0f) * 0.002f;
    shiftAmount += timeShift;
    
    // 各色チャンネルを少しずつずらしてサンプリング
    float r = gTexture.Sample(gSampler, uv + float2(shiftAmount, 0.0f)).r;
    float g = gTexture.Sample(gSampler, uv).g;
    float b = gTexture.Sample(gSampler, uv - float2(shiftAmount, 0.0f)).b;
    
    output.color = float4(r, g, b, 1.0f);
    
    return output;
}