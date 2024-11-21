[vk::binding(0, 0)]
cbuffer Offsets
{
    float2 offset;
};

[vk::binding(1, 0)]
Sampler2D tex;

struct vertexInfo
{
    [vk::location(0)] float2 position;
    [vk::location(1)] float3 color;
}

struct v2p
{
    float2 uv : TEXCOORD0;
    float3 color : COLOR0;
    float4 position : SV_Position;
}

[shader("vertex")]
v2p vertex(vertexInfo input) {
    v2p output;
    output.position = float4(input.position + offset, 0.0, 1.0);
    output.color = input.color;
    output.uv = input.position;
    return output;
}

[shader("pixel")]
float4 pixel(v2p input)
{
    // return float4(input.uv, 0.0, 1.0);
    return tex.Sample(input.uv);
}
