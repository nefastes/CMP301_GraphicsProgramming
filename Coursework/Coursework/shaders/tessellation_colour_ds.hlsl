// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
};

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uv : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
	float3 vertexPosition = float3(0.f, 0.f, 0.f);
    OutputType output;
 
	// Determine the position of the new vertex
	float3 v1 = lerp(patch[0].position, patch[2].position, uv.y);
	float3 v2 = lerp(patch[1].position, patch[3].position, uv.y);
	vertexPosition = lerp(v1, v2, uv.x);
		    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Send the input color into the pixel shader.
    output.colour = patch[0].colour;

    return output;
}

