#include "Fullscreen.hlsli"

struct NoiseMaterial
{
    // 16バイト境界に厳密に合わせた構造体
    float32_t4 color; // 16 bytes (0-15)
    
    // 次の16バイトブロック：int値は4バイトだが、16バイト境界で区切る
    int32_t enableLighting; // 4 bytes  (16-19) 
    int32_t useLambertianReflectance; // 4 bytes  (20-23)
    float32_t2 padding1; // 8 bytes  (24-31) 16バイト境界まで埋める
    
    // 64バイトの4x4行列（4つの16バイトブロック）
    float32_t4x4 uvTransform; // 64 bytes (32-95)
    
    // 次の16バイトブロック：ノイズパラメータ1
    float32_t time; // 4 bytes  (96-99)
    float32_t noiseIntensity; // 4 bytes  (100-103)
    float32_t blockSize; // 4 bytes  (104-107)
    float32_t animationSpeed; // 4 bytes  (108-111)
    
    // 次の16バイトブロック：ノイズカラー
    float32_t4 noiseColor; // 16 bytes (112-127)
    
    // 次の16バイトブロック：残りのパラメータ
    float32_t colorVariation; // 4 bytes  (128-131)
    float32_t blockDensity; // 4 bytes  (132-135)
    float32_t2 padding2; // 8 bytes  (136-143) 16バイト境界まで埋める
};

ConstantBuffer<NoiseMaterial> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

// ランダム値を生成する関数
float random(float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // テクスチャサンプリング
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 基本はテクスチャの色を使用
    output.color = textureColor;
    
    // ノイズ強度が0以下なら何もしない
    if (gMaterial.noiseIntensity <= 0.0)
    {
        return output;
    }
    if (input.texcoord.x < 0.5)
    {
        return output;
        
    }
    // ブロックサイズに基づいてグリッド計算
    float2 screenSize = float2(1280.0, 720.0); // 画面サイズ（固定）
    float2 blockCount = screenSize / gMaterial.blockSize;
    float2 blockPos = floor(input.texcoord * blockCount);
    
    // 時間ベースでアニメーション
    float animTime = gMaterial.time * gMaterial.animationSpeed;
    float timeStep = floor(animTime * 10.0); // 0.1秒ごとに更新
    
    // このブロック位置でのランダム値を生成
    float randomValue = random(blockPos + timeStep);
    
    // ブロック密度に基づいてノイズを表示するかどうか決定
    if (randomValue < gMaterial.blockDensity)
    {
        // ブロック内での色のバリエーション
        float colorVar = random(blockPos + timeStep * 1.3) - 0.5;
        float3 noiseCol = gMaterial.noiseColor.rgb;
        noiseCol += colorVar * gMaterial.colorVariation;
        noiseCol = saturate(noiseCol); // 0-1にクランプ
        
        // 元のテクスチャ色とノイズ色をブレンド
        output.color.rgb = lerp(textureColor.rgb, noiseCol, gMaterial.noiseIntensity);
    }
    
    return output;
}