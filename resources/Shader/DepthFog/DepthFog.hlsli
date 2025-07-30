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
// 改良された深度線形化関数
float32_t DepthToWorldDistanceLinearized(float32_t depth)
{
     // 深度バッファは通常非線形分布のため線形化が必要
    // 簡易的な線形化: near=0.1, far=100.0 として計算
    float32_t nearPlane = 0.1f;
    float32_t farPlane = 100.0f;
    
    // 深度値が1.0の場合は明示的にfarPlane距離とする
    if (depth >= 0.999999f)
    {
        return farPlane;
    }
    
    // より正確な線形化計算
    float32_t z_ndc = depth * 2.0f - 1.0f; // [0,1] → [-1,1]
    float32_t linearDepth = (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z_ndc * (farPlane - nearPlane));
    
    // 範囲チェック
    linearDepth = clamp(linearDepth, nearPlane, farPlane);
    
    return linearDepth;
}

// または、さらにシンプルな方法
float32_t DepthToWorldDistanceSimple(float32_t depth)
{
     // 深度バッファは通常非線形分布のため線形化が必要
    // 簡易的な線形化: near=0.1, far=100.0 として計算
    float32_t nearPlane = 0.1f;
    float32_t farPlane = 100.0f;
    // 深度値が1.0に非常に近い場合はfarPlaneとして扱う
    if (depth >= 0.9999f)
    {
        return farPlane;
    }
    
    // 標準的な線形化
    float32_t linearDepth = nearPlane * farPlane / (farPlane - depth * (farPlane - nearPlane));
    
    // 明示的にfarPlane以下にクランプ
    return min(linearDepth, farPlane);
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