#include "resources/Shader/Grayscale/Grayscale.hlsli"

FullscreenVertexOutput main(FullscreenVertexInput input)
{
    FullscreenVertexOutput output;

    output.position = input.position;
    output.texcoord = input.texcoord;
    
    return output;
}