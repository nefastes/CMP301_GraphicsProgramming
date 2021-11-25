// Simple colour pixel shader

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
    return float4(1.f, 0.f, 0.f, 1.f);
}