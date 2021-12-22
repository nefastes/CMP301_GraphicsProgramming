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
#define CACHESIZE (NUMTHREADS_X + 2 * gBlurRadius) * (NUMTHREADS_Y + 2 * gBlurRadius)
groupshared float4 gCache[CACHESIZE];

void apply_threshold(uint index)
{
	float4 out_colour = float4(0.f, 0.f, 0.f, 0.f);
	float4 pixel = gCache[index];
	float avg = (pixel.x + pixel.y + pixel.z) / 3.f;
	out_colour += (avg > bloom_threshold) * pixel;
	gCache[index] = out_colour;
}

[numthreads(NUMTHREADS_X, NUMTHREADS_Y, 1)] //This defines how many threads there are within a tread group. Dispatch(x,y,z) defines how many threads groups wil be created.
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)	//uint3 SV_GroupID, uint SV_GroupIndex unused
{
	float4 out_colour = float4(0.f, 0.f, 0.f, 0.f);
	float4 blurColor = float4(0, 0, 0, 0);

	//// Cache the input textures
	// Chache horizontal data
	uint cache_index_middle = groupThreadID.x + gBlurRadius + (groupThreadID.y + gBlurRadius) * NUMTHREADS_X;
	uint cache_index_left = groupThreadID.x + (groupThreadID.y + gBlurRadius) * NUMTHREADS_X;
	uint cache_index_right = groupThreadID.x + 2 * gBlurRadius + (groupThreadID.y + gBlurRadius) * NUMTHREADS_X;
	uint cache_index_top = groupThreadID.x + gBlurRadius + groupThreadID.y * NUMTHREADS_X;
	uint cache_index_bottom = groupThreadID.x + gBlurRadius + (groupThreadID.y + 2 * gBlurRadius) * NUMTHREADS_X;
	
	if (groupThreadID.x < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[cache_index_left] = gInputScene[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= NUMTHREADS_X - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = min(dispatchThreadID.x + gBlurRadius, gInputScene.Length.x - 1);
		gCache[cache_index_right] = gInputScene[int2(x, dispatchThreadID.y)];
	}
	// Chache vertical data
	if (groupThreadID.y < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[cache_index_top] = gInputScene[int2(dispatchThreadID.x, y)];
	}
	if (groupThreadID.y >= NUMTHREADS_Y - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + gBlurRadius, gInputScene.Length.y - 1);
		gCache[cache_index_bottom] = gInputScene[int2(dispatchThreadID.x, y)];
	}
	// Clamp out of bound samples that occur at image borders.
	gCache[cache_index_middle] = gInputScene[min(dispatchThreadID.xy, gInputScene.Length.xy - 1)];
	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();
	
	//Threshold
	if (groupThreadID.x < gBlurRadius)
	{
		//apply_threshold(cache_index_left);
	}
	if (groupThreadID.x >= NUMTHREADS_X - gBlurRadius)
	{
		//apply_threshold(cache_index_right);
	}
	if (groupThreadID.y < gBlurRadius)
	{
		//apply_threshold(cache_index_top);
	}
	if (groupThreadID.y >= NUMTHREADS_Y - gBlurRadius)
	{
		//apply_threshold(cache_index_bottom);
	}
	apply_threshold(cache_index_middle);
	GroupMemoryBarrierWithGroupSync();
	
	////Horizontal blur
	//[unroll]
	//for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	//{
	//	//int k = groupThreadID.x + gBlurRadius + i + (groupThreadID.y + gBlurRadius) * NUMTHREADS_X;

	//	blurColor += gWeights[i + gBlurRadius] * gCache[cache_index_middle + i];
	//}
	//GroupMemoryBarrierWithGroupSync();
	//gCache[cache_index_middle] = blurColor;
	//blurColor = float4(0.f, 0.f, 0.f, 0.f);
	//GroupMemoryBarrierWithGroupSync();
	
	////Vertical blur
	//[unroll]
	//for (int j = -gBlurRadius; j <= gBlurRadius; ++j)
	//{
	//	//int k = groupThreadID.y + gBlurRadius + j;
	//	int k = groupThreadID.x + gBlurRadius + (groupThreadID.y + gBlurRadius + j) * NUMTHREADS_X;

	//	blurColor += gWeights[j + gBlurRadius] * gCache[cache_index_middle + j * NUMTHREADS_X];
	//}
	//GroupMemoryBarrierWithGroupSync();
	//gCache[cache_index_middle] = blurColor;
	
	//Combine result
	gOutput[dispatchThreadID.xy] = /*gInputScene[dispatchThreadID.xy] + */gCache[cache_index_middle];
}