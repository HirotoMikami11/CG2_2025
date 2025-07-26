#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

FullscreenVertexOutput main(FullscreenVertexInput input)
{
    FullscreenVertexOutput output;
    
    // 入力座標はすでにNDC座標系（-1～1）で指定されているため
    // 変換行列による計算は不要で、そのまま出力
    output.position = input.position;
    output.texcoord = input.texcoord;
    
    return output;
}