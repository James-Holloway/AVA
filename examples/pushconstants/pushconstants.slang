struct PushConstants
{
    float4x4 mvp;
    float4 color;
};

[push_constant]
PushConstants pc;

struct VSIn
{
    [vk::location(0)] float3 position;
    [vk::location(1)] float3 normal;
};

struct VSOut
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
};

[shader("vertex")]
VSOut vertex(VSIn input)
{
    VSOut output;
    output.position = mul(pc.mvp, float4(input.position, 1.0));
    output.normal = input.normal;
    return output;
}

[shader("fragment")]
float4 fragment(VSOut input)
{
    float4 color = pc.color;
    // Lighting
    float3 N = normalize(input.normal);
	float3 L = normalize(float3(1.0, 2.0, 4.0));
    float diffuse = dot(N, L) * 0.5 + 0.5;
    diffuse = clamp(diffuse, 0.2, 1.0);

    color.rgb *= diffuse;

    return color;
}