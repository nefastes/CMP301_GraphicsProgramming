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
	matrix lightViewMatrix[N_LIGHTS * 6];
	matrix lightProjectionMatrix[N_LIGHTS];
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
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos[N_LIGHTS * 6] : TEXCOORD3;
    //float4 depthPosition[N_LIGHTS * 6] : TEXCOORD27;
};

float getHeight(float2 uv)
{
    return heightMap.SampleLevel(Sampler, uv, 0).x * 20.f;
}

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uv : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
	float3 vertexPosition = float3(0.f, 0.f, 0.f);
	float2 texCoord;
    OutputType output;
 
    // Determine the position of the new vertex.
	vertexPosition.xz =
        patch[0].position.xz * (1.f - uv.x) * (1.f - uv.y) +
        patch[1].position.xz * uv.x * (1.f - uv.y) +
        patch[2].position.xz * (1.f - uv.x) * uv.y +
        patch[3].position.xz * uv.x * uv.y;
        
	texCoord =
		patch[0].tex * (1.f - uv.x) * (1.f - uv.y) +
        patch[1].tex * uv.x * (1.f - uv.y) +
        patch[2].tex * (1.f - uv.x) * uv.y +
        patch[3].tex * uv.x * uv.y;
	
	//Get the height of the vertex from the heightmap
    vertexPosition.y = getHeight(texCoord);
		    
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

		// Store the position value in a second input value for depth value calculations.
        //output.depthPosition[i] = output.lightViewPos[i];
    }

	//Calculate the position of the vertex viewed by the camera
	output.viewVector = patch[0].cameraPosition - output.worldPosition;
	output.viewVector = normalize(output.viewVector);

    // Send the input tex into the pixel shader.
	output.tex = texCoord;
    
    // Send the input normal into the pixel shader.
	output.normal = mul(patch[0].normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);

    return output;
}

