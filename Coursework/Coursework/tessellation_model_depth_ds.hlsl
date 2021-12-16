// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

#define N_LIGHTS 4

Texture2D heightMap : register(t0);
Texture2D normalMap : register(t1);
SamplerState Sampler : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer SettingsBuffer : register(b1)
{
    float height_amplitude;
    float3 padding;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 cameraPosition : TEXCOORD1;
};

struct OutputType
{
    float4 position : SV_POSITION;
};

[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvw : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
    float3 vertexPosition = float3(0.f, 0.f, 0.f);
    OutputType output;
 
    // Determine the position of the new vertex.
    vertexPosition =
        patch[0].position * uvw.x +
        patch[1].position * uvw.y +
        patch[2].position * uvw.z;
		    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}

