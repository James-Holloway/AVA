[vk::binding(0,0)]
cbuffer UBO
{
    float4x4 view;
    float4x4 projection;
    float4x4 model;
};

[vk::binding(1, 0)]
Sampler2D textures[16];

struct VSIn
{
    [vk::location(0)] float3 position;
    [vk::location(1)] float3 normal;
    [vk::location(2)] float2 uv;
    [vk::location(3)] int textureIndex;
}

struct VSOut
{
    float4 position : SV_Position;
    [vk::location(0)] float3 normal : NORMAL;
    [vk::location(1)] float2 uv : TEXCOORD0;
    [vk::location(2)] int textureIndex : TEXCOORD1;
}

struct FSOut
{
    // swapchain : 0
    float4 albedo : SV_Target1;
    float4 normal : SV_Target2;
}

[shader("vertex")]
VSOut vertex(VSIn input)
{
    float4x4 mvp = mul(mul(projection, view), model);
    VSOut output;
    output.position = mul(mvp, float4(input.position, 1.0));
    output.normal = mul(model, float4(input.normal, 0.0)).xyz;
    output.uv = float2(input.uv.x, 1.0 - input.uv.y);
    output.textureIndex = input.textureIndex;
    return output;
}

[shader("fragment")]
FSOut fragment(VSOut input)
{
    float4 albedo = float4(1.0, 1.0, 1.0, 1.0);
    if (input.textureIndex >= 0)
    {
        albedo *= textures[input.textureIndex].Sample(input.uv);
    }

    FSOut output;
    output.albedo = albedo;
    output.normal = float4(input.normal, 1.0);
    return output;
}