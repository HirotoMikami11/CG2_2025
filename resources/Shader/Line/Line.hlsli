struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t4 color : COLOR0; // 頂点色をピクセルシェーダーに渡す
};