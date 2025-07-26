// 頂点シェーダー入力構造体
struct FullscreenVertexInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};

// 頂点シェーダー出力構造体
struct FullscreenVertexOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

// ピクセルシェーダー出力構造体
struct FullscreenPixelOutput
{
    float32_t4 color : SV_TARGET0;
};