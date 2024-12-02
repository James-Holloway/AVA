[vk::binding(0,0)]
Sampler2D resultTexture;

struct VSOut
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
}

[shader("vertex")]
VSOut vertex(uint vertexID : SV_VertexID)
{
    VSOut output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.uv * 2.0 - 1.0, 0.0, 1.0);
    return output;
}

[shader("fragment")]
float4 fragment(VSOut input) : SV_Target0
{
    return resultTexture.Sample(input.uv.xy);
}