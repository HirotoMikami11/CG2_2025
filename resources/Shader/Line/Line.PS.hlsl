#include "resources/Shader/Line/Line.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // 頂点から渡された色をそのまま使用
    output.color = input.color;
    
    return output;
}