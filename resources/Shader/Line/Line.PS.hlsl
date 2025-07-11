#include "resources/Shader/Line/Line.hlsli"

struct LineMaterial
{
    float32_t4 color; // 基本色
};

ConstantBuffer<LineMaterial> gMaterial : register(b0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // マテリアルの色を使用（頂点からの色は無視）
    output.color = gMaterial.color;
    
    return output;
}