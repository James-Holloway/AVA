struct vertexInfo
{
    [vk::location(0)] float3 position;
    [vk::location(1)] float3 normal;
    [vk::location(2)] float2 uv;
}

[vk::binding(0,0)]
cbuffer UBO
{
    float4x4 mvp;
} ubo;

struct v2p
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
}

[shader("vertex")]
v2p vertex(vertexInfo input) {
    v2p output;
    output.position = mul(ubo.mvp, float4(input.position, 1.0));
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}

[shader("pixel")]
float4 pixel(v2p input)
{
    return float4(input.normal * 0.5 + 0.5, 1.0);
}
