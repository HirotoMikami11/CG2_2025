Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);
StructuredBuffer<float> gScrollOffsets : register(t1);

cbuffer ScrollControl : register(b1)
{
    float2 direction; // (1,0)=X方向, (0,1)=Y方向, (-1,1)=斜めとかもOK
}

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    int y = int(input.position.y);
    float offset = gScrollOffsets[y];
    //float2 scrolledUV = input.uv + float2(offset, 0);
    float2 scrolledUV = input.uv + direction * offset;
    return gTexture.Sample(gSampler, scrolledUV);
}
