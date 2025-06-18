struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

//grayScale
struct GrayscaleParameters
{
    float32_t4 color; 
    
    float32_t time;             // 時間（アニメーション用）
    float32_t grayIntensity;    // グレースケールの度合い(0~1の範囲で表現)
    float32_t unused;          // パディング
    float32_t unused2;          // パディング（16バイト境界合わせ）
};