// Simple throughput vertex shader. Work being done by the geometry shader.

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

InputType main(InputType input)
{
	return input;
}