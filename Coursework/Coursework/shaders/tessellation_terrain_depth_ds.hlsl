// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

#define N_LIGHTS 4

Texture2D heightMap : register(t0);
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
	float2 texture_scale;
    float padding;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
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

float getHeight(float2 uv)
{
    return heightMap.SampleLevel(Sampler, uv, 0).x * height_amplitude;
}

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uv : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
	float3 vertexPosition = float3(0.f, 0.f, 0.f);
	float2 texCoord;
    OutputType output;
 
    // Determine the position of the new vertex.
	vertexPosition =
        patch[0].position * (1.f - uv.x) * (1.f - uv.y) +
        patch[1].position * uv.x * (1.f - uv.y) +
        patch[2].position * (1.f - uv.x) * uv.y +
        patch[3].position * uv.x * uv.y;
        
	texCoord =
		patch[0].tex * (1.f - uv.x) * (1.f - uv.y) +
        patch[1].tex * uv.x * (1.f - uv.y) +
        patch[2].tex * (1.f - uv.x) * uv.y +
        patch[3].tex * uv.x * uv.y;
	
	//Get the height of the vertex from the heightmap
	vertexPosition.y += getHeight(texCoord * texture_scale);
		    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    return output;
}

