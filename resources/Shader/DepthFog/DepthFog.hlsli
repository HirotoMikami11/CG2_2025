#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// 深度フォグパラメータ構造体
struct DepthFogParameters
{
    float32_t4 fogColor; // フォグの色 (RGB + 強度)
    
    float32_t fogNear; // フォグ開始距離
    float32_t fogFar; // フォグ終了距離（完全にフォグ色になる距離）
    float32_t fogDensity; // フォグの密度（0.0f〜1.0f）
    float32_t time; // アニメーション用時間
};

// 深度値から実際のワールド距離を計算する関数
float32_t DepthToWorldDistanceLinearized(float32_t depth)
{
    // 深度バッファは通常非線形分布のため線形化が必要
    // 簡易的な線形化: near=0.1, far=1000.0 として計算
    float32_t nearPlane = 0.1f;
    float32_t farPlane = 1000.0f;
    
    // 深度値を線形距離に変換
    float32_t linearDepth = nearPlane * farPlane / (farPlane - depth * (farPlane - nearPlane));
    return linearDepth;
}

// 線形フォグの計算関数
float32_t CalculateLinearFog(float32_t distance, float32_t fogNear, float32_t fogFar)
{
    // 距離が近距離より手前の場合はフォグなし
    if (distance <= fogNear)
    {
        return 0.0f;
    }
    
    // 距離が遠距離より奥の場合は完全にフォグ
    if (distance >= fogFar)
    {
        return 1.0f;
    }
    
    // 線形補間でフォグファクターを計算
    return (distance - fogNear) / (fogFar - fogNear);
}