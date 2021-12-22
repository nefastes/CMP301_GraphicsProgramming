// triangle_gs
// Geometry shader that generates a triangle for every vertex.

#define N_LIGHTS 4

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct InputType
{
	float4 position : POSITION;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float4 colour : COULOUR0;
};

//Take a tesselated triangle as input, and create a sprout of grass on it
[maxvertexcount(3)]
void main(triangle InputType input[3], inout TriangleStream<OutputType> triangleStream)
{
	OutputType output;
	
	float4 displacement = float4(0.f, 1.f, 0.f, 0.f);
	float3 normal = input[0].normal;
	float4 vertex = input[0].position + displacement;
	output.position = mul(vertex, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.colour = float4(0.f, 1.f, 0.f, 1.f);
	triangleStream.Append(output);
	
	displacement = float4(-.0125f, 0.f, 0.f, 0.f);
	vertex = input[0].position + displacement;
	output.position = mul(vertex, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.colour = float4(0.f, 1.f, 0.f, 1.f);
	triangleStream.Append(output);
	
	displacement = float4(.0125f, 0.f, 0.f, 0.f);
	vertex = input[0].position + displacement;
	output.position = mul(vertex, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.colour = float4(0.f, 1.f, 0.f, 1.f);
	triangleStream.Append(output);
	
	triangleStream.RestartStrip();
}