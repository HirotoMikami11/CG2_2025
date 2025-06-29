struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

// ライングリッチパラメータ
struct LineGlitchParameters
{
    float32_t time;                 // 時間（アニメーション用）
    float32_t noiseIntensity;       // グリッチの強度
    float32_t noiseInterval;         // ノイズが起こる頻度(0近ければ近いほど起こりやすく、増やすほど起こりにくくなる)
    float32_t animationSpeed;        // 全体に適応する時間(内部時間にかける値)
};