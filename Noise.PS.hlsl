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
    float32_t time; // 4 bytes  (96-99)                                             //経過時間
    float32_t noiseIntensity; // 4 bytes  (100-103)                                 //全体のノイズの強さ(それぞれ増やす場合、意味が変わる場合がある)
    float32_t noiseInterval; // 4 bytes  (104-107)                                  //ノイズが起こる頻度(0近ければ近いほど起こりやすく、増やすほど起こりにくくなる)
    float32_t animationSpeed; // 4 bytes  (108-111)                                 //全体に適応する時間(内部時間にかける値)
    
    // 次の16バイトブロック：ノイズカラー
    float32_t4 noiseColor; // 16 bytes (112-127)                                    //未使用(小分けにして4つパラメータを用意できる)
    
    // 次の16バイトブロック：残りのパラメータ
    float32_t blackIntensity; // 4 bytes  (128-131)                                 //グレースケールの補間率(0に近いほど画像の色,1ほどグレースケール画像)
    float32_t colorNoiseInternsity; // 4 bytes  (132-135)                                   //色ずらしの強さ
    float32_t2 blockDivision; // 8 bytes  (136-143) 16バイト境界まで埋める             //ブロックずらしの画面分割数(小さいほど比率的には大きくなる)
};

ConstantBuffer<NoiseMaterial> gMaterial : register(b0);

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
    float t = frac(gMaterial.time); // 0.0〜1.0で時間をループ
    float t0 = floor(gMaterial.time);
    float t1 = t0 + 1.0;

    // 2つのランダム値を取得（異なる時間ステップ）
    float r0 = hash(st + t0);
    float r1 = hash(st + t1);

    // 線形補間
    return lerp(r0, r1, t);
    
    
   // return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
    
}

// グレースケール変換関数（人間の視覚に基づく加重平均）
float3 toGrayscale(float3 color)
{
    float luminance = dot(color, float3(0.299, 0.587, 0.114));
    return float3(luminance, luminance, luminance);
}

// カラーとグレースケールをイージングする関数
//元の色とイージングのtの入れる
//0に近ければ近いほど元の色に、1に近いほどグレースケールに。
float3 applyGrayscale(float3 color, float t)
{
    float3 grayscale = toGrayscale(color);
    return lerp(color, grayscale, t);
}

float4 DrawBlockGrid(float2 uv, float4 originalColor)
{
    // ブロック内での相対位置を計算（0.0～1.0の範囲）
    float2 blockPos = frac(uv * float2(gMaterial.blockDivision.x, gMaterial.blockDivision.y));
    
    // 線の太さを設定（調整可能）
    float lineWidth = 0.01; // 1%の太さ（0.005で細く、0.02で太く）
    
    // 境界線かどうかを判定
    bool verticalLine = (blockPos.x < lineWidth) || (blockPos.x > (1.0 - lineWidth));
    bool horizontalLine = (blockPos.y < lineWidth) || (blockPos.y > (1.0 - lineWidth));
    
    // 線の色を設定
    float4 lineColor = float4(1.0, 0.0, 0.0, 1.0); // 赤色の線
    
    // 境界線の場合は線の色、そうでなければ元の色
    if (verticalLine || horizontalLine)
    {
        return lineColor;
    }
    else
    {
        return originalColor;
    }
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
    
    //半分は普通に描画する
    //if (input.texcoord.x < 0.5)
    //{
    //    return output;
    //}
    
    
    float2 uv = input.texcoord;
    float animTime = gMaterial.time * gMaterial.animationSpeed;
 
    // グリッチの強度（断続的に発生）
    float glitchTrigger = step(gMaterial.noiseInterval, random(floor(animTime * 8.0))); //stepは0.8が適切な値
    float glitchIntensity = gMaterial.noiseIntensity * glitchTrigger;
 
    if (glitchIntensity > 0.0)
    {
        //// 1. 水平ピクセルずれ（Horizontal displacement）
        //float lineNoise = random(floor(uv.y * 200.0) + floor(animTime * 20.0));
        //float displacement = (lineNoise - 0.5) * glitchIntensity * 0.1;
        //uv.x += displacement;
        
        //// 2. ブロック状の破損
        //float2 blockPos = floor(uv * float2(gMaterial.blockDivision.x, gMaterial.blockDivision.y)); //float2(40,20)画面を40x20で分割
        //float blockRandomVal = random(blockPos + floor(animTime * 5.0)); //animTime*5.0は50くらいにしてもいいかも
   
        //if (blockRandomVal > 0.95)//個々の値もパラメータにできる
        //{
        // // ブロック単位で画像をずらす
        //    float blockShift = (random(blockPos + 1.0) - 0.5) * 0.8; //0.2がシフト量の大きさ。個々のパラメータを用意すれば変えられる
        //    uv.x += blockShift * glitchIntensity; //ノイズの強度で動かしてるので、別の値を用意しないとずれ幅がピクセルずれと連動してわかりずらいかも、だが、blockShift出かけるので結局上の列をパラメータとして持つべき
        //    uv.y += blockShift * glitchIntensity; //ノイズの強度で動かしてるので、別の値を用意しないとずれ幅がピクセルずれと連動してわかりずらいかも、だが、blockShift出かけるので結局上の列をパラメータとして持つべき
        //}
        
        
        // 3. RGBチャンネル分離（色収差）
        float rgbShift = gMaterial.colorNoiseInternsity * 0.02;
        float2 redUV = uv + float2(rgbShift, 0.0);
        float2 greenUV = uv;
        float2 blueUV = uv - float2(rgbShift, 0.0);
     
        //チャンネル別にサンプリング
        float r = gTexture.Sample(gSampler, redUV).r;
        float g = gTexture.Sample(gSampler, greenUV).g;
        float b = gTexture.Sample(gSampler, blueUV).b;
        float a = gTexture.Sample(gSampler, uv).a;
     
        output.color = float4(r, g, b, a);
            
        //output.color = gTexture.Sample(gSampler,uv);
        
             ////gTextureの仕様理解するためにコメントアウトを外す
        //float r = gTexture.Sample(gSampler, uv).r;
        //float g = gTexture.Sample(gSampler, uv).g;
        //float b = gTexture.Sample(gSampler, uv).b;
        //float a = gTexture.Sample(gSampler, uv).a;
        //output.color = float4(0, 0, b, a);
        
        
        
        
        
        //   // 4. 走査線ノイズ
        //float scanLine = sin(uv.y * 800.0 + animTime * 10.0) * 0.5 + 0.5;
        //float scanNoise = random(float2(floor(uv.y * 400.0), floor(animTime * 30.0)));
        //if (scanNoise > 0.98)
        //{
        //    output.color.rgb = lerp(output.color.rgb, float3(scanLine, scanLine, scanLine), 0.3);
        //}
     
        ////. デジタルアーティファクト（時々画像が反転）
        //if (blockRandomVal > 0.99)
        //{
        //    float2 flippedUV = float2(1.0 - uv.x, uv.y);
        //    output.color = gTexture.Sample(gSampler, flippedUV);
        //}
            
        //// 6.グレースケールを適用
        //output.color.rgb = applyGrayscale(output.color.rgb, gMaterial.blackIntensity); //この1はtなのでパラメータにできる
        
        
   

        ////それぞれ出力するときにコメントアウトを外す
        //output.color = gTexture.Sample(gSampler, uv);
        
       //// ブロック境界線を適用（グリッチ効果と合成）
       // output.color = DrawBlockGr 9id(input.texcoord, output.color);
        //// デバッグ：ブロック効果を色で可視化
        //if (blockRandomVal > 0.99)
        //{
        //    output.color = float4(1.0, 0.0, 0.0, 1.0); // 反転ブロックを赤で表示
        //    return output;
        //}
        //else if (blockRandomVal > 0.95)
        //{
        //    output.color = float4(0.0, 1.0, 0.0, 1.0); // シフトブロックを緑で表示
        //    return output;
        //}
    }
    else
    {
     // グリッチが発生していない時は通常の画像
        output.color = gTexture.Sample(gSampler, uv);

    }

    
    return output;
}