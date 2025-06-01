struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

// グリッチエフェクト用のパラメータ
struct GlitchMaterial
{
    float32_t4 color;
    int32_t enableLighting; // 使わない
    int32_t useLambertianReflectance; // 使わない
    float32_t4x4 uvTransform;
    
    // グリッチエフェクト用パラメータ
    float32_t rgbShiftStrength; // RGBシフトの強度
    float32_t time; // 時間（アニメーション用）
    float32_t glitchIntensity; // 全体的なグリッチ強度
    float32_t unused; // パディング用（16バイト境界合わせ）
};