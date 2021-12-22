Texture2D gInputScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer Settings : register(b0)
{
	float bloom_threshold;
	float3 padding;
};

cbuffer Constant_Settings
{
	static float gWeights[11] =
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
	static const int gBlurRadius = 5;
};

#define NUMTHREADS_X 16
#define NUMTHREADS_Y 16
#define HORIZONTAL_CACHESIZE NUMTHREADS_X + 2 * gBlurRadius
#define VERTICAL_CACHESIZE NUMTHREADS_Y + 2 * gBlurRadius
groupshared float4 gHorizontalCache[HORIZONTAL_CACHESIZE];
groupshared float4 gVerticalCache[VERTICAL_CACHESIZE];

void threshold_horizontal(uint index)
{
	float4 out_colour = float4(0.f, 0.f, 0.f, 0.f);
	float4 pixel = gHorizontalCache[index];
	float avg = (pixel.x + pixel.y + pixel.z) / 3.f;
	out_colour += (avg > bloom_threshold) * pixel;
	gHorizontalCache[index] = out_colour;
}

[numthreads(NUMTHREADS_X, NUMTHREADS_Y, 1)] //This defines how many threads there are within a tread group. Dispatch(x,y,z) defines how many threads groups wil be created.
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)	//uint3 SV_GroupID, uint SV_GroupIndex unused
{
	float4 blurColour = float4(0.f, 0.f, 0.f, 0.f);

	//// Cache the input textures
	// Chache horizontal data
	if (groupThreadID.x < gBlurRadius)
	{
	// Clamp out of bound samples that occur at image borders.
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gHorizontalCache[groupThreadID.x] = gInputScene[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= NUMTHREADS_X - gBlurRadius)
	{
	// Clamp out of bound samples that occur at image borders.
		int x = min(dispatchThreadID.x + gBlurRadius, gInputScene.Length.x - 1);
		gHorizontalCache[groupThreadID.x + 2 * gBlurRadius] = gInputScene[int2(x, dispatchThreadID.y)];
	}
	// Clamp out of bound samples that occur at image borders.
	gHorizontalCache[groupThreadID.x + gBlurRadius] = gInputScene[min(dispatchThreadID.xy, gInputScene.Length.xy - 1)];
	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//Threshold
	if (groupThreadID.x < gBlurRadius)
	{
		//threshold_horizontal(groupThreadID.x);
	}
	if (groupThreadID.x >= NUMTHREADS_X - gBlurRadius)
	{
		//threshold_horizontal(groupThreadID.x + 2 * gBlurRadius);
	}
	threshold_horizontal(groupThreadID.x + gBlurRadius);
	//Wait for all the threshold to be applied
	GroupMemoryBarrierWithGroupSync();
	
	//Horizontal blur
	[unroll]
	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		//int k = groupThreadID.x + gBlurRadius + i + (groupThreadID.y + gBlurRadius) * NUMTHREADS_X;

		blurColour += gWeights[i + gBlurRadius] * gHorizontalCache[groupThreadID.x + gBlurRadius + i];
	}
	GroupMemoryBarrierWithGroupSync();
	gOutput[dispatchThreadID.xy] = blurColour;
	
	// Chache vertical data
	if (groupThreadID.y < gBlurRadius)
	{
	// Clamp out of bound samples that occur at image borders.
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gVerticalCache[groupThreadID.y] = gOutput[int2(dispatchThreadID.x, y)];
	}
	if (groupThreadID.y >= NUMTHREADS_Y - gBlurRadius)
	{
	// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + gBlurRadius, gOutput.Length.y - 1);
		gVerticalCache[groupThreadID.y + 2 * gBlurRadius] = gOutput[int2(dispatchThreadID.x, y)];
	}
	// Clamp out of bound samples that occur at image borders.
	gVerticalCache[groupThreadID.y + gBlurRadius] = gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)];
	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	////Vertical blur
	blurColour = float4(0.f, 0.f, 0.f, 0.f);
	[unroll]
	for (int j = -gBlurRadius; j <= gBlurRadius; ++j)
	{
		blurColour += gWeights[j + gBlurRadius] * gVerticalCache[groupThreadID.y + gBlurRadius + j];
	}
	GroupMemoryBarrierWithGroupSync();
	gVerticalCache[groupThreadID.y + gBlurRadius] = blurColour;
	
	//Combine result
	gOutput[dispatchThreadID.xy] = gInputScene[dispatchThreadID.xy] + gVerticalCache[groupThreadID.y + gBlurRadius];
}