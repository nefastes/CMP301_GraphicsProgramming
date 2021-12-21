//Compute shader to output the pixels above the threshold value to the RWTexture2D

Texture2D gInput1 : register(t0);
Texture2D gInput2 : register(t1);
RWTexture2D<float4> gOutput : register(u0);

[numthreads(16, 16, 1)] //This defines how many threads there are within a tread group. Dispatch(x,y,z) defines how many threads groups wil be created.
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)	//uint3 SV_GroupID, uint SV_GroupIndex unused
{
	gOutput[dispatchThreadID.xy] = gInput1[dispatchThreadID.xy] + gInput2[dispatchThreadID.xy]; //dispatchThreadID = pixel location
}