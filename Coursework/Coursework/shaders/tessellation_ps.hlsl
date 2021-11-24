// Tessellation pixel shader
// Output colour passed to stage.

struct InputType
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
};

float4 main(InputType input) : SV_TARGET
{
    //return input.colour;
	return float4(1.f, 0.f, 0.f, 1.f);
}