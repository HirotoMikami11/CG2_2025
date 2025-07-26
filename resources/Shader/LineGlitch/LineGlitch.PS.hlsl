#include "resources/Shader/LineGlitch/LineGlitch.hlsli"

ConstantBuffer<LineGlitchParameters> LineGlitchParameter : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;

    // テクスチャサンプリング
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 基本はテクスチャの色を使用
    output.color = textureColor;
    
    // ノイズ強度が0以下なら何もしない
    if (LineGlitchParameter.noiseIntensity <= 0.0)
    {
        return output;
    }
    
    float2 uv = input.texcoord;
    float animTime = LineGlitchParameter.time * LineGlitchParameter.animationSpeed;
 
    // グリッチの強度（断続的に発生）
    float glitchTrigger = step(LineGlitchParameter.noiseInterval, random(floor(animTime * 8.0), LineGlitchParameter.time));
    float glitchIntensity = LineGlitchParameter.noiseIntensity * glitchTrigger;
 
    if (glitchIntensity > 0.0)
    {
        // 水平ピクセルずれ（Horizontal displacement）
        float lineNoise = random(floor(uv.y * 400.0) + floor(animTime * 20.0), LineGlitchParameter.time);
        float displacement = (lineNoise - 0.5) * glitchIntensity * 0.1;
        uv.x += displacement;
        
        // テクスチャサンプリング（ずらされたUV座標で）
        output.color = gTexture.Sample(gSampler, uv);
    }
    else
    {
        // グリッチが発生していない時は通常の画像
        output.color = gTexture.Sample(gSampler, uv);
    }

    return output;
}