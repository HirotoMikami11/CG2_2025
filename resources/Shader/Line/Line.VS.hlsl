#include "resources/Shader/Line/Line.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0; // 使用しないが構造体の互換性のため
    float32_t3 normal : NORMAL0; // 使用しないが構造体の互換性のため
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // 位置変換
    output.position = mul(input.position, gTransformationMatrix.WVP);
    
    // 固定色（白）を設定 - マテリアルは使用しない
    output.color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return output;
}