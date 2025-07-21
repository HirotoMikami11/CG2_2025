#include "resources/Shader/DepthOfField/DepthOfField.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    // normalは被写界深度エフェクトでは不要
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.texcoord = input.texcoord;
    return output;
}