#include "resources/Shader/LineGlitch/LineGlitch.hlsli"

ConstantBuffer<LineGlitchParameters> LineGlitchParameter : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

///timeを使いつつ、値が大きくならないようにするために使うハッシュ関数
float hash(float2 st)
{
    //元のランダム関数
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

// ランダム値を生成する関数
float random(float2 st)
{
    // 時間を使ってランダム値を生成する場合
    float t = frac(LineGlitchParameter.time); // 0.0〜1.0で時間をループ
    float t0 = floor(LineGlitchParameter.time);
    float t1 = t0 + 1.0;

    // 2つのランダム値を取得（異なる時間ステップ）
    float r0 = hash(st + t0);
    float r1 = hash(st + t1);

    // 線形補間
    return lerp(r0, r1, t);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

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
    float glitchTrigger = step(LineGlitchParameter.noiseInterval, random(floor(animTime * 8.0)));
    float glitchIntensity = LineGlitchParameter.noiseIntensity * glitchTrigger;
 
    if (glitchIntensity > 0.0)
    {
        // 水平ピクセルずれ（Horizontal displacement）
        float lineNoise = random(floor(uv.y * 400.0) + floor(animTime * 20.0));
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