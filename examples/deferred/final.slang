[vk::input_attachment_index(0), vk::binding(1,0)]
SubpassInput depth;
[vk::input_attachment_index(1), vk::binding(2,0)]
SubpassInput albedo;
[vk::input_attachment_index(2), vk::binding(3,0)]
SubpassInput normal;

[vk::binding(0,0)]
cbuffer UBO
{
    float4x4 view;
    float4x4 projection;
    float4x4 model;
}

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
    float3 color = albedo.SubpassLoad().xyz;
    if (input.uv.x < 0.5)
    {
        color = abs(normal.SubpassLoad().xyz);
    }

    return float4(color, 1.0);
}