#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// グレースケールパラメータ構造体
struct GrayscaleParameters
{
    float32_t4 color; // 基本色（通常は白色 1,1,1,1）
    
    float32_t time; // 時間（アニメーション用）
    float32_t grayIntensity; // グレースケールの度合い（0.0f～1.0f）
    float32_t unused1; // パディング
    float32_t unused2; // パディング（16バイト境界合わせ）
};

// グレースケール変換関数（加重平均）
float32_t3 ToGrayscale(float32_t3 color)
{
    // 人間の視覚特性に基づいた重み付け
    float32_t luminance = dot(color, float32_t3(0.299f, 0.587f, 0.114f));
    return float32_t3(luminance, luminance, luminance);
}

// カラーとグレースケールを線形補間する関数
float32_t3 ApplyGrayscale(float32_t3 originalColor, float32_t grayIntensity)
{
    float32_t3 grayscaleColor = ToGrayscale(originalColor);
    return lerp(originalColor, grayscaleColor, grayIntensity);
}