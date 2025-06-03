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
    
    
    //if (input.texcoord.x < 0.5)
    //{
    //    return output;
        
    //}
    
    
    float2 uv = input.texcoord;
    float animTime = gMaterial.time * gMaterial.animationSpeed;
 
    // グリッチの強度（断続的に発生）
    float glitchTrigger = step(0.5, random(floor(animTime * 8.0)));
    float glitchIntensity = gMaterial.noiseIntensity * glitchTrigger;
 
    if (glitchIntensity > 0.0)
    {
        // 1. 水平ピクセルずれ（Horizontal displacement）
        float lineNoise = random(floor(uv.y * 200.0) + floor(animTime * 20.0));
        float displacement = (lineNoise - 0.5) * glitchIntensity * 0.1;
        uv.x += displacement;
     
         // 2. ブロック状の破損
        float2 blockPos = floor(uv * float2(40.0, 20.0));
        float blockRandom = random(blockPos + floor(animTime * 5.0));
        if (blockRandom > 0.95)
        {
         // ブロック単位で画像をずらす
            float blockShift = (random(blockPos + 1.0) - 0.5) * 0.2;
            uv.x += blockShift * glitchIntensity;
        }
     
          // 3. RGBチャンネル分離（色収差）
        float rgbShift = glitchIntensity * 0.02;
        float2 redUV = uv + float2(rgbShift, 0.0);
        float2 greenUV = uv;
        float2 blueUV = uv - float2(rgbShift, 0.0);
     
          // チャンネル別にサンプリング
        float r = gTexture.Sample(gSampler, redUV).r;
        float g = gTexture.Sample(gSampler, greenUV).g;
        float b = gTexture.Sample(gSampler, blueUV).b;
        float a = gTexture.Sample(gSampler, uv).a;
     
        output.color = float4(r, g, b, a);
     
           // 4. 走査線ノイズ
        float scanLine = sin(uv.y * 800.0 + animTime * 10.0) * 0.5 + 0.5;
        float scanNoise = random(float2(floor(uv.y * 400.0), floor(animTime * 30.0)));
        if (scanNoise > 0.98)
        {
            output.color.rgb = lerp(output.color.rgb, float3(scanLine, scanLine, scanLine), 0.3);
        }
     
           // 5. デジタルアーティファクト（時々画像が反転）
        if (blockRandom > 0.99)
        {
            float2 flippedUV = float2(1.0 - uv.x, uv.y);
            output.color = gTexture.Sample(gSampler, flippedUV);
        }
    }
    else
    {
     // グリッチが発生していない時は通常の画像
        output.color = gTexture.Sample(gSampler, uv);
    }
    
    return output;
}