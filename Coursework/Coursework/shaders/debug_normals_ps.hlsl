// Simple colour pixel shader

struct InputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
    return float4(abs(input.normal.xyz), 1.f);
}