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
	matrix lightViewMatrix[N_LIGHTS * 6];
	matrix lightProjectionMatrix[N_LIGHTS];
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
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos[N_LIGHTS * 6] : TEXCOORD3;
};

float getHeight(float2 uv)
{
    return heightMap.SampleLevel(Sampler, uv, 0).x * height_amplitude;
}

[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvw : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
    float3 vertexPosition, normal;
	float2 texCoord;
    OutputType output;
 
    // Determine the position of the new vertex.
	vertexPosition =
        patch[0].position * uvw.x +
        patch[1].position * uvw.y +
        patch[2].position * uvw.z;
	
    normal =
        patch[0].normal * uvw.x +
        patch[1].normal * uvw.y +
        patch[2].normal * uvw.z;
        
	texCoord =
		patch[0].tex * uvw.x +
        patch[1].tex * uvw.y +
        patch[2].tex * uvw.z;
	
	// Send the input normal into the pixel shader.
    //normal = normalMap.SampleLevel(Sampler, texCoord, 0).xyz;
    output.normal = mul(normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

	//Get the height of the vertex from the heightmap
  //  float3 direction = patch[0].normal * getHeight(texCoord);
  //  float4x4 translation =
  //  {
  //      1.f, 0.f, 0.f, 0.f,
		//0.f, 1.f, 0.f, 0.f,
		//0.f, 0.f, 1.f, 0.f,
		//direction.x, direction.y, direction.z, 1.f
  //  };
  //  vertexPosition = mul(float4(vertexPosition, 1.f), translation);
		    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
	output.worldPosition = output.position;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertice as viewed by the light source.
	for (int i = 0; i < N_LIGHTS * 6; ++i)
	{
		output.lightViewPos[i] = mul(float4(vertexPosition, 1.f), worldMatrix);
		output.lightViewPos[i] = mul(output.lightViewPos[i], lightViewMatrix[i]);
		output.lightViewPos[i] = mul(output.lightViewPos[i], lightProjectionMatrix[i / 6]);
    }

	//Calculate the position of the vertex viewed by the camera
	output.viewVector = patch[0].cameraPosition - output.worldPosition;
	output.viewVector = normalize(output.viewVector);

    // Send the input tex into the pixel shader.
    output.tex = texCoord;

    return output;
}

