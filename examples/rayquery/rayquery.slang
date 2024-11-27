struct VSIn
{
    [vk::location(0)] float3 position;
    [vk::location(1)] float3 normal;
}

struct VSOut
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD0;
}

[vk::binding(0, 0)]
cbuffer UBO
{
    float4x4 view;
    float4x4 projection;
    float4x4 model;
    float4 lightPosition;
} ubo;

[vk::binding(1, 0)]
RaytracingAccelerationStructure tlas;

[shader("vertex")]
VSOut vertex(VSIn input)
{
    float4x4 mvp = mul(mul(ubo.projection, ubo.view), ubo.model);
    VSOut output;
    output.position = mul(mvp, float4(input.position, 1.0));
    output.normal = mul(ubo.model, float4(input.normal, 0.0)).xyz;
    output.worldPosition = mul(ubo.model, float4(input.position, 1.0)).xyz;
    return output;
}

[shader("fragment")]
float4 fragment(VSOut input)
{
    // Lighting
    float3 N = normalize(input.normal);
	float3 L = normalize(ubo.lightPosition.xyz - input.worldPosition);
    float diffuse = dot(N, L) * 0.5 + 0.5;
    diffuse = clamp(diffuse, 0.2, 1.0);

    // Ray query shadow
    float shadow = 1.0f;
    
    RayDesc ray;
    ray.Origin = input.worldPosition;
    ray.Direction = L;
    ray.TMin = 0.05f;
    ray.TMax = length(ubo.lightPosition.xyz - input.worldPosition.xyz) - 0.01f;
    RayQuery<RAY_FLAG_NONE> query;
    let rayFlags = RAY_FLAG_NONE;
    query.TraceRayInline(tlas, rayFlags, 0xff, ray);
    query.Proceed();
    if (query.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        shadow = 0.1f;
    }

    return float4(float3(diffuse, diffuse, diffuse) * shadow, 1.0);
}