// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

#define N_LIGHTS 4

Texture2D heightMap : register(t0);
SamplerState Sampler : register(s0);

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
};

struct OutputType
{
	float4 position : POSITION;
	float3 normal : NORMAL;
};

float getHeight(float2 uv)
{
	return heightMap.SampleLevel(Sampler, uv, 0).x * height_amplitude;
}

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uv : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
	float3 vertexPosition = float3(0.f, 0.f, 0.f);
	float3 normal;
	float2 texCoord;
	OutputType output;
 
	// Determine the position of the new vertex
	float3 v1 = lerp(patch[0].position, patch[2].position, uv.y);
	float3 v2 = lerp(patch[1].position, patch[3].position, uv.y);
	vertexPosition = lerp(v1, v2, uv.x);
        
	float2 t1 = lerp(patch[0].tex, patch[2].tex, uv.y);
	float2 t2 = lerp(patch[1].tex, patch[3].tex, uv.y);
	texCoord = lerp(t1, t2, uv.x);
	
	float3 n1 = lerp(patch[0].normal, patch[2].normal, uv.y);
	float3 n2 = lerp(patch[1].normal, patch[3].normal, uv.y);
	normal = lerp(n1, n2, uv.x);
	
	//Get the height of the vertex from the heightmap
	vertexPosition.y += getHeight(texCoord * texture_scale); //the normal is up, no need to consider it here
	
	output.position = float4(vertexPosition, 1.0f);
	output.normal = normal;

	return output;
}