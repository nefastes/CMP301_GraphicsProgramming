//Compute shader to output the pixels above the threshold value to the RWTexture2D

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer Settings : register(b0)
{
	float bloom_threshold;
	float3 padding;
};

[numthreads(16, 16, 1)] //This defines how many threads there are within a tread group. Dispatch(x,y,z) defines how many threads groups wil be created.
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)	//uint3 SV_GroupID, uint SV_GroupIndex unused
{
	// Sample each pixel and output if above threshold, otherwise output black
	float4 out_colour = float4(0.f, 0.f, 0.f, 0.f);
	
	float4 pixel = gInput[dispatchThreadID.xy];
	float avg = (pixel.x + pixel.y + pixel.z) / 3.f;
	
	out_colour += (avg > bloom_threshold) * pixel;

	gOutput[dispatchThreadID.xy] = out_colour; //dispatchThreadID = pixel location
}