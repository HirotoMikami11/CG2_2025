#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// ライングリッチパラメータ
struct LineGlitchParameters
{
    float32_t time; // 時間（アニメーション用）
    float32_t noiseIntensity; // グリッチの強度
    float32_t noiseInterval; // ノイズが起こる頻度(0近ければ近いほど起こりやすく、増やすほど起こりにくくなる)
    float32_t animationSpeed; // 全体に適応する時間(内部時間にかける値)
};

// timeを使いつつ、値が大きくならないようにするために使うハッシュ関数
float hash(float2 st)
{
    // 元のランダム関数
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

// ランダム値を生成する関数
float random(float2 st, float time)
{
    // 時間を使ってランダム値を生成する場合
    float t = frac(time); // 0.0〜1.0で時間をループ
    float t0 = floor(time);
    float t1 = t0 + 1.0;

    // 2つのランダム値を取得（異なる時間ステップ）
    float r0 = hash(st + t0);
    float r1 = hash(st + t1);

    // 線形補間
    return lerp(r0, r1, t);
}