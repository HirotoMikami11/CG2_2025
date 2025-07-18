struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

// 深度フォグパラメータ構造体
struct DepthFogParameters
{
    float32_t4 fogColor; // フォグの色
    
    float32_t fogNear; // フォグ開始距離
    float32_t fogFar; // フォグ終了距離（完全にフォグ色になる距離）
    float32_t fogDensity; // フォグの密度（0.0f〜1.0f）
    float32_t time; // アニメーション用時間
};

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

// 深度値から世界座標距離への変換（超シンプル版）
float32_t DepthToWorldDistance(float32_t depth)
{
    // 極めてシンプル：深度値をそのまま0-100の距離に線形変換
    return depth * 100.0f;
}