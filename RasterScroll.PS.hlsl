Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);
StructuredBuffer<float> gScrollOffsets : register(t1);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    int y = int(input.position.y);
    float offset = gScrollOffsets[y];
    float2 scrolledUV = input.uv + float2(offset, 0);
    return gTexture.Sample(gSampler, scrolledUV);
}
