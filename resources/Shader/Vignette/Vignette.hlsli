#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// ビネットパラメータ構造体
struct VignetteParameters
{
    float32_t4 vignetteColor; // ビネットの色（通常は黒色 0,0,0,1）
    
    float32_t time; // 時間（アニメーション用）
    float32_t vignetteStrength; // ビネットの強度（0.0f～1.0f）
    float32_t vignetteRadius; // ビネットの半径（0.0f～1.0f）
    float32_t vignetteSoftness; // ビネットの柔らかさ（0.0f～1.0f）
};

// ビネット効果を計算する関数
float32_t CalculateVignette(float32_t2 texcoord, float32_t radius, float32_t softness)
{
    // 画面中央からの距離を計算（0.0f～1.0f）
    float32_t2 center = float32_t2(0.5f, 0.5f);
    float32_t2 offset = texcoord - center;
    
    // アスペクト比を考慮した距離計算
    // 正方形での距離になるように調整
    float32_t distance = length(offset);
    
    // ビネット効果の計算
    float32_t vignette = 1.0f - smoothstep(radius, radius + softness, distance);
    
    return vignette;
}

// ビネット効果を適用する関数
float32_t3 ApplyVignette(float32_t3 originalColor, float32_t2 texcoord, VignetteParameters params)
{
    // ビネット係数を計算
    float32_t vignetteAmount = CalculateVignette(texcoord, params.vignetteRadius, params.vignetteSoftness);
    
    // 時間による揺らぎを追加（オプション）
    float32_t timeEffect = 1.0f + sin(params.time * 2.0f) * 0.05f; // 軽微な脈動効果
    vignetteAmount *= timeEffect;
    
    // ビネット色と元の色を混合
    float32_t3 finalColor = lerp(
        params.vignetteColor.rgb, // ビネット色（通常は黒）
        originalColor, // 元の色
        vignetteAmount // ビネット係数
    );
    
    // 強度を適用
    return lerp(originalColor, finalColor, params.vignetteStrength);
}