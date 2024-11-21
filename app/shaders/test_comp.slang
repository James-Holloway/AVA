[vk::binding(0, 0)]
cbuffer UBO
{
    float time;
} ubo;

struct Offsets
{
    float2 offset;
};

[vk::binding(1, 0)]
RWStructuredBuffer<Offsets> offsets;

[shader("compute")]
void compute()
{
    offsets[0].offset = float2(sin(ubo.time), cos(ubo.time)) * 0.5;
}
