struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

// 被写界深度パラメータ構造体
struct DepthOfFieldParameters
{
    float32_t4 focusParams; // x:焦点距離, y:焦点範囲, z:ボケ強度, w:未使用
    
    float32_t focusDistance; // 焦点距離
    float32_t focusRange; // 焦点範囲（この範囲内はシャープ）
    float32_t blurStrength; // ボケの強度
    float32_t time; // アニメーション用時間
};

// 深度値から世界座標距離への変換
float32_t DepthToWorldDistanceLinearized(float32_t depth)
{
    // 深度バッファは通常非線形分布
    // 簡易的な線形化: near=0.1, far=1000.0 として計算
    float32_t nearPlane = 0.1f;
    float32_t farPlane = 1000.0f;
    
    // 深度値を線形距離に変換
    float32_t linearDepth = nearPlane * farPlane / (farPlane - depth * (farPlane - nearPlane));
    return linearDepth;
}

// 焦点からの距離に基づいてボケ強度を計算
float32_t CalculateBlurFactor(float32_t distance, float32_t focusDistance, float32_t focusRange)
{
    // 焦点距離からのずれを計算
    float32_t focusOffset = abs(distance - focusDistance);
    
    // 焦点範囲内の場合はボケなし
    if (focusOffset <= focusRange * 0.5f)
    {
        return 0.0f;
    }
    
    // 焦点範囲外の場合は距離に応じてボケ強度を計算
    float32_t blurStart = focusRange * 0.5f;
    float32_t blurFactor = (focusOffset - blurStart) / (focusRange * 2.0f);
    
    // ボケ強度を0.0〜1.0の範囲でクランプ
    return saturate(blurFactor);
}