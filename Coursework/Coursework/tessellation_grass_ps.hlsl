struct InputType
{
	float4 position : SV_POSITION;
	float4 colour : COULOUR0;
};

float4 main(InputType input) : SV_TARGET
{
	return input.colour;
}