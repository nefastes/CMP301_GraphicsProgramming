//Compute shader to output the pixels above the threshold value to the RWTexture2D

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer Settings : register(b0)
{
	float bloom_threshold;
	float3 padding;
};

//This defines how many threads there are within a tread group. Dispatch(x,y,z) defines how many threads groups will be created.
[numthreads(16, 16, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	//Sample each pixel and output if above threshold, otherwise output black
	float4 out_colour = float4(0.f, 0.f, 0.f, 0.f);
	//First get the pixel colour
	float4 pixel = gInput[dispatchThreadID.xy]; //dispatchThreadID = pixel location
	//Calculate the brightness by simply adding its compenents
	float brightness = pixel.x + pixel.y + pixel.z;
	//If the brightness is above our threshold, output it to the texture, else ouput black
	out_colour += (brightness > bloom_threshold) * pixel;
	gOutput[dispatchThreadID.xy] = out_colour;
}