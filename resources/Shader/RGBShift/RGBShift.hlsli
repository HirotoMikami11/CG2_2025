#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// RGBシフトパラメータ
struct RGBShiftParameters
{
    float32_t rgbShiftStrength; // RGBシフトの強度
    float32_t time; // 時間（アニメーション用）
    float32_t unused1; // パディング
    float32_t unused2; // パディング（16バイト境界合わせ）
};