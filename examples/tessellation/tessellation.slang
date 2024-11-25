[vk::binding(0, 0)]
cbuffer UBO
{
    float4x4 viewProjection;
    float4x4 model;
    float tessAlpha;
    float tessLevel;
} ubo;

struct VSIn
{
    [vk::location(0)] float3 position;
    [vk::location(1)] float3 normal;
    [vk::location(2)] float2 uv;
};

struct VSOut
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PNPatch
{
    float pnPatch[10];

    __init()
    {
        [unroll]
        for (int i = 0; i < 10; i++)
        {
            pnPatch[i] = 0;
        }
    }

    __init(float pn[10])
    {
        pnPatch = pn;
    }

    __subscript(int index) -> float
    {
        get { return pnPatch[index]; }
        set { pnPatch[index] = newValue; }
    }

    #define PN_PROPERTY(name, index) \
        property name : float \
    { \
        get { return pnPatch[index]; } \
        set { pnPatch[index] = newValue; } \
    }
    
    PN_PROPERTY(b210, 0)
    PN_PROPERTY(b120, 1)
    PN_PROPERTY(b021, 2)
    PN_PROPERTY(b012, 3)
    PN_PROPERTY(b102, 4)
    PN_PROPERTY(b201, 5)
    PN_PROPERTY(b111, 6)
    PN_PROPERTY(n110, 7)
    PN_PROPERTY(n011, 8)
    PN_PROPERTY(n101, 9)
};

struct HSOut
{
    float4 position : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
    float pnPatch[10] : TEXCOORD5;
};

struct ConstantsHSOut
{
    float TessLevelOuter[4] : SV_TessFactor;
    float TessLevelInner[2] : SV_InsideTessFactor;
};

struct DSOut
{
    float4 position : SV_Position;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
};

// Vertex passthrough
[shader("vertex")]
VSOut vertex(VSIn input)
{
    VSOut output;
    output.position = float4(input.position, 1.0);
    output.normal = input.normal;
    output.uv = input.uv;
    return output;
}

// Hull / Tess control
ConstantsHSOut hullConstants()
{
    ConstantsHSOut output;
    output.TessLevelOuter[0] = ubo.tessLevel;
    output.TessLevelOuter[1] = ubo.tessLevel;
    output.TessLevelOuter[2] = ubo.tessLevel;
    output.TessLevelInner[0] = ubo.tessLevel;
    return output;
}

float wij(float4 iPos, float3 iNormal, float4 jPos)
{
    return dot(jPos.xyz - iPos.xyz, iNormal);
}

float vij(float4 iPos, float3 iNormal, float4 jPos, float3 jNormal)
{
    float3 PjMinusPi = jPos.xyz - iPos.xyz;
    float3 NiPlusNj = iNormal + jNormal;
    return 2.0 * dot(PjMinusPi, NiPlusNj) / dot(PjMinusPi, PjMinusPi);
}

float wij(VSOut i, VSOut j)
{
    return wij(i.position, i.normal, j.position);
}

float vij(VSOut i, VSOut j)
{
    return vij(i.position, i.normal, j.position, j.normal);
}

[shader("hull")]
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("hullConstants")]
// [maxtessfactor(20.0f)] // expects a string when documentation says float
HSOut hull(InputPatch<VSOut, 3> patch, int invocationID : SV_OutputControlPointID)
{
    HSOut output;
    output.position = patch[invocationID].position;
    output.normal = patch[invocationID].normal;
    output.uv = patch[invocationID].uv;

    // Set base
    float P0 = patch[0].position[invocationID];
    float P1 = patch[1].position[invocationID];
    float P2 = patch[2].position[invocationID];
    float N0 = patch[0].normal[invocationID];
    float N1 = patch[1].normal[invocationID];
    float N2 = patch[2].normal[invocationID];

    // Compute control points
    PNPatch pnPatch;
    pnPatch.b210 = ((2.0 * P0) + P1 - wij(patch[0], patch[1]) * N0) / 3.0;
    pnPatch.b120 = ((2.0 * P1) + P0 - wij(patch[1], patch[0]) * N1) / 3.0;
    pnPatch.b021 = ((2.0 * P1) + P2 - wij(patch[1], patch[2]) * N1) / 3.0;
    pnPatch.b012 = ((2.0 * P2) + P1 - wij(patch[2], patch[1]) * N2) / 3.0;
    pnPatch.b102 = ((2.0 * P2) + P0 - wij(patch[2], patch[0]) * N2) / 3.0;
    pnPatch.b201 = ((2.0 * P0) + P2 - wij(patch[0], patch[2]) * N0) / 3.0;
    float E = (pnPatch.b210 + pnPatch.b120 + pnPatch.b021 + pnPatch.b012 + pnPatch.b102 + pnPatch.b201) / 6.0;
    float V = (P0 + P1 + P2) / 3.0;
    pnPatch.b111 = E + (E - V) * 0.5;
    pnPatch.n110 = N0 + N1 - vij(patch[0], patch[1]) * (P1 - P0);
    pnPatch.n011 = N1 + N2 - vij(patch[1], patch[2]) * (P2 - P1);
    pnPatch.n101 = N2 + N0 - vij(patch[2], patch[0]) * (P0 - P2);

    output.pnPatch = pnPatch.pnPatch;

    return output;
}

// Domain / Tess evaluation
float barycentrics(float3 tessCoord, float a, float b, float c)
{
    return tessCoord[2] * a + tessCoord[0] * b + tessCoord[1] * c;
}
float2 barycentrics(float3 tessCoord, float2 a, float2 b, float2 c)
{
    return tessCoord[2] * a + tessCoord[0] * b + tessCoord[1] * c;
}
float3 barycentrics(float3 tessCoord, float3 a, float3 b, float3 c)
{
    return tessCoord[2] * a + tessCoord[0] * b + tessCoord[1] * c;
}
float4 barycentrics(float3 tessCoord, float4 a, float4 b, float4 c)
{
    return tessCoord[2] * a + tessCoord[0] * b + tessCoord[1] * c;
}

[shader("domain")]
DSOut domain(ConstantsHSOut input, float3 tessCoord : SV_DomainLocation, const OutputPatch<HSOut, 3> patch)
{
    PNPatch pnPatch[3];
    pnPatch[0] = PNPatch(patch[0].pnPatch);
    pnPatch[1] = PNPatch(patch[1].pnPatch);
    pnPatch[2] = PNPatch(patch[2].pnPatch);

    DSOut output;
    
    float3 uvw = tessCoord;
    float3 uvwSquared = uvw * uvw;
    float3 uvwCubed = uvwSquared * uvw;

    // Extract control points
    float3 b210 = float3(pnPatch[0].b210, pnPatch[1].b210, pnPatch[2].b210);
    float3 b120 = float3(pnPatch[0].b120, pnPatch[1].b120, pnPatch[2].b120);
    float3 b021 = float3(pnPatch[0].b021, pnPatch[1].b021, pnPatch[2].b021);
    float3 b012 = float3(pnPatch[0].b012, pnPatch[1].b012, pnPatch[2].b012);
    float3 b102 = float3(pnPatch[0].b102, pnPatch[1].b102, pnPatch[2].b102);
    float3 b201 = float3(pnPatch[0].b201, pnPatch[1].b201, pnPatch[2].b201);
    float3 b111 = float3(pnPatch[0].b111, pnPatch[1].b111, pnPatch[2].b111);

    // Extract control normals
    float3 n110 = normalize(float3(pnPatch[0].n110, pnPatch[1].n110, pnPatch[2].n110));
    float3 n011 = normalize(float3(pnPatch[0].n011, pnPatch[1].n011, pnPatch[2].n011));
    float3 n101 = normalize(float3(pnPatch[0].n101, pnPatch[1].n101, pnPatch[2].n101));

    // Texcoords
    output.uv = barycentrics(tessCoord, patch[0].uv, patch[1].uv, patch[2].uv);

    // Normal
    float3 barNormal = barycentrics(tessCoord, patch[0].normal, patch[1].normal, patch[2].normal);
    float3 pnNormal = barycentrics(uvwSquared, patch[0].normal, patch[1].normal, patch[2].normal) + n110*uvw[2]*uvw[0] + n011*uvw[0]*uvw[1] + n101*uvw[2]*uvw[1];
    output.normal = lerp(barNormal, pnNormal, ubo.tessAlpha);

    float3 barPos = barycentrics(tessCoord, patch[0].position.xyz, patch[1].position.xyz, patch[2].position.xyz);

    float3 uvwSquared3 = uvwSquared * 3.0;
    float3 pnPos = barycentrics(uvwCubed, patch[0].position.xyz, patch[1].position.xyz, patch[2].position.xyz)
                + b210 * uvwSquared3[2] * uvw[0]
                + b120 * uvwSquared3[0] * uvw[2]
                + b201 * uvwSquared3[2] * uvw[1]
                + b021 * uvwSquared3[0] * uvw[1]
                + b102 * uvwSquared3[1] * uvw[2]
                + b012 * uvwSquared3[1] * uvw[0]
                + b111 * 6.0 * uvw[0] * uvw[1] * uvw[2];

    float3 pos = lerp(barPos, pnPos, ubo.tessAlpha);
    output.position = mul(mul(ubo.viewProjection, ubo.model), float4(pos, 1.0));

    return output;
}

// Fragment
[shader("fragment")]
float4 fragment(DSOut input) : SV_Target
{
	float4 color = float4(input.normal * 0.5 + 0.5, 1.0f);

    // Lighting
    float3 N = normalize(input.normal);
	float3 L = normalize(float3(0.0, -4.0, 4.0));
    float diffuse = max(dot(N, L), 0.0);
    diffuse = clamp(diffuse, 0.2, 1.0);

    color.rgb *= diffuse * 1.45;

	return color;
}
