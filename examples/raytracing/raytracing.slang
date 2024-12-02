[vk::binding(0, 0)]
RaytracingAccelerationStructure tlas;

[vk::binding(1, 0)]
[format("rgba16f")]
RWTexture2D resultTexture;

[vk::binding(2, 0)]
cbuffer UBO
{
    float screenWidth;
    float screenHeight;
    int frameIndex;
    int padding;
    float4x4 viewProjection;
    float4x4 viewInverse;
    float4x4 projectionInverse;
} ubo;

struct Vertex
{
    float4 position;
    float4 normal;
}

struct Object
{
    Vertex* vertices;
    uint32_t* indices;
    uint32_t indexWidth;
}

[vk::binding(3, 0)]
restrict StructuredBuffer<Object> objectBuffer;

struct RayPayload
{
    float4 color;
}

[shader("raygeneration")]
void raygen()
{
    uint2 threadIndex = DispatchRaysIndex().xy;
    if (threadIndex.x >= (int)ubo.screenWidth) return;
    if (threadIndex.y >= (int)ubo.screenHeight) return;
    uint2 threadSize = DispatchRaysDimensions().xy;

    float2 pixelCenter = float2(threadIndex);
    float2 inUV = pixelCenter / float2(threadSize);
    float2 d = inUV * 2.0 - 1.0;

    float4 origin = mul(ubo.viewInverse, float4(0.0, 0.0, 0.0, 1.0));
    const float4 target = mul(ubo.projectionInverse, float4(d.xy, 1.0, 1.0));
    float4 direction = mul(ubo.viewInverse, float4(target.xyz, 0.0));

    RayDesc ray;
    ray.Origin = origin.xyz;
    ray.Direction = direction.xyz;
    ray.TMin = 0.01;
    ray.TMax = 100.0;

    RayPayload payload;
    payload.color = float4(0, 0, 0, 0);
    TraceRay(tlas, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

    resultTexture[threadIndex.xy] = payload.color;
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float4(0, 0, 0, 1);
}

[shader("closesthit")]
void closestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    float3 hitLocation = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    /*
    let instanceID = InstanceID();
    let primitiveID = PrimitiveIndex();
    let indexWidth = objectBuffer[instanceID].indexWidth;

    uint3 face;
    if (indexWidth == 32)
    {
        let indices = objectBuffer[instanceID].indices;
        face.x = indices[3 * primitiveID + 0];
        face.y = indices[3 * primitiveID + 1];
        face.z = indices[3 * primitiveID + 2];
    }
    else
    {
        let indices = (uint16_t*)objectBuffer[instanceID].indices;
        face.x = indices[3 * primitiveID + 0];
        face.y = indices[3 * primitiveID + 1];
        face.z = indices[3 * primitiveID + 2];
    }

    let vertices =  objectBuffer[instanceID].vertices;

    Vertex v0 = vertices[face.x];
    Vertex v1 = vertices[face.y];
    Vertex v2 = vertices[face.z];

    let normal = v0.normal.xyz * barycentrics.x + v1.normal.xyz * barycentrics.y + v2.normal.xyz * barycentrics.z;
    let worldNormal = normalize(mul(float3x3(ObjectToWorld()), normal));

    payload.color = float4(worldNormal, 1.0);
    */
    payload.color = float4(barycentrics, 1.0);
}
