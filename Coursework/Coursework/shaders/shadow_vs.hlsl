#define N_LIGHTS 4
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix[N_LIGHTS * 6];
	matrix lightProjectionMatrix[N_LIGHTS];
};

cbuffer CameraBuffer : register(b1)
{
	float3 cameraPosition;
	float padding;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
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


OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.worldPosition = output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertice as viewed by the light source.
	for (int i = 0; i < N_LIGHTS * 6; ++i)
	{
		output.lightViewPos[i] = mul(input.position, worldMatrix);
		output.lightViewPos[i] = mul(output.lightViewPos[i], lightViewMatrix[i]);
		output.lightViewPos[i] = mul(output.lightViewPos[i], lightProjectionMatrix[i / 6]);
	}

	//Calculate the position of the vertex viewed by the camera
	output.viewVector = cameraPosition - output.worldPosition;
	output.viewVector = normalize(output.viewVector);

	//Texture and normal
	output.tex = input.tex;
	output.normal = mul(input.normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);

	return output;
	}