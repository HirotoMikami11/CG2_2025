Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

// ラインごとのスクロール量
StructuredBuffer<float> gScrollOffsets : register(t1);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    int y = int(input.position.y); // スキャンラインY
    float offsetX = gScrollOffsets[y]; // スクロール量
    float2 scrolledUV = input.uv + float2(offsetX, 0.0f);
    return gTexture.Sample(gSampler, scrolledUV);
}
