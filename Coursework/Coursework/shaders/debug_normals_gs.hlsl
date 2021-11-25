// triangle_gs
// Geometry shader that generates a triangle for every vertex.

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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
    float3 normal : NORMAL;
};

[maxvertexcount(4)]
void main(triangle InputType input[3], inout LineStream<OutputType> lineStream)
{
	OutputType output;
    
    //for (int i = 0; i < 3; ++i)
    //{
    //    output.position = input[i].position;
    //    output.position = mul(output.position, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    lineStream.Append(output);
    //    output.position = input[i].position + float4(input[i].normal, 0.f);
    //    output.position = mul(output.position, worldMatrix);
    //    output.position = mul(output.position, viewMatrix);
    //    output.position = mul(output.position, projectionMatrix);
    //    lineStream.Append(output);
    //    lineStream.RestartStrip();
    //}
    
    float4 vertex = (input[0].position + input[1].position + input[2].position) / 3.f;
    float4 normal = float4((input[0].normal + input[1].normal + input[2].normal) / 3.f, 0.f);
    output.normal = normal.xyz;
    
    output.position = mul(vertex, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    lineStream.Append(output);
    output.position = vertex + normal;
    output.position = mul(output.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    lineStream.Append(output);
    lineStream.RestartStrip();
}