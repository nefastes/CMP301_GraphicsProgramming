Texture2D gInputScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer Settings : register(b0)
{
	float bloom_threshold;
	int num_blur_passes;
	float2 padding;
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
#define HORIZONTAL_CACHESIZE (NUMTHREADS_X + 2 * gBlurRadius)
#define VERTICAL_CACHESIZE (NUMTHREADS_Y + 2 * gBlurRadius)
groupshared float4 gHorizontalCache[HORIZONTAL_CACHESIZE][NUMTHREADS_Y];
groupshared float4 gVerticalCache[VERTICAL_CACHESIZE][NUMTHREADS_X];

[numthreads(NUMTHREADS_X, NUMTHREADS_Y, 1)] //This defines how many threads there are within a tread group. Dispatch(x,y,z) defines how many threads groups wil be created.
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)	//uint3 SV_GroupID, uint SV_GroupIndex unused
{
	float4 out_colour = float4(0.f, 0.f, 0.f, 0.f);
	float4 blurColor = float4(0, 0, 0, 0);

	//// Cache the input texture and apply the threshold on that pixel
	float4 pixel = gInputScene[min(dispatchThreadID.xy, gInputScene.Length.xy - 1)];
	float avg = (pixel.x + pixel.y + pixel.z) / 3.f;
	out_colour += (avg > bloom_threshold) * pixel;
	gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)] = out_colour;
	
	for (int blur_pass = 0; blur_pass < num_blur_passes; ++blur_pass)
	{
		if (groupThreadID.x < gBlurRadius)
		{
		// Clamp out of bound samples that occur at image borders.
			int x = max(dispatchThreadID.x - gBlurRadius, 0);
			gHorizontalCache[groupThreadID.x][groupThreadID.y] = gOutput[int2(x, dispatchThreadID.y)];
		}
		if (groupThreadID.x >= NUMTHREADS_X - gBlurRadius)
		{
		// Clamp out of bound samples that occur at image borders.
			int x = min(dispatchThreadID.x + gBlurRadius, gOutput.Length.x - 1);
			gHorizontalCache[groupThreadID.x + 2 * gBlurRadius][groupThreadID.y] = gOutput[int2(x, dispatchThreadID.y)];
		}
		// Clamp out of bound samples that occur at image borders.
		gHorizontalCache[groupThreadID.x + gBlurRadius][groupThreadID.y] = gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)];
		// Wait for all threads to finish.
		GroupMemoryBarrierWithGroupSync();
	
		//Horizontal blur
		[unroll]
		for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
		{
			blurColor += gWeights[i + gBlurRadius] * gHorizontalCache[groupThreadID.x + gBlurRadius + i][groupThreadID.y];
		}
		//GroupMemoryBarrierWithGroupSync();
		//gHorizontalCache[groupThreadID.x + gBlurRadius][groupThreadID.y] = blurColor;
		gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)] = blurColor;
		blurColor = float4(0.f, 0.f, 0.f, 0.f);
		GroupMemoryBarrierWithGroupSync();
	
		// Chache vertical data
		if (groupThreadID.y < gBlurRadius)
		{
		// Clamp out of bound samples that occur at image borders.
			int y = max(dispatchThreadID.y - gBlurRadius, 0);
			gVerticalCache[groupThreadID.y][groupThreadID.x] = gOutput[int2(dispatchThreadID.x, y)];
		}
		if (groupThreadID.y >= NUMTHREADS_Y - gBlurRadius)
		{
		// Clamp out of bound samples that occur at image borders.
			int y = min(dispatchThreadID.y + gBlurRadius, gOutput.Length.y - 1);
			gVerticalCache[groupThreadID.y + 2 * gBlurRadius][groupThreadID.x] = gOutput[int2(dispatchThreadID.x, y)];
		}
		// Clamp out of bound samples that occur at image borders.
		gVerticalCache[groupThreadID.y + gBlurRadius][groupThreadID.x] = gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)];
		// Wait for all threads to finish.
		GroupMemoryBarrierWithGroupSync();
	
		//Vertical blur
		[unroll]
		for (int j = -gBlurRadius; j <= gBlurRadius; ++j)
		{
			blurColor += gWeights[j + gBlurRadius] * gVerticalCache[groupThreadID.y + gBlurRadius + j][groupThreadID.x];
		}
		gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)] = blurColor;
		blurColor = float4(0.f, 0.f, 0.f, 0.f);
		GroupMemoryBarrierWithGroupSync();
	}
	
	//Combine result
	gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)] = gInputScene[min(dispatchThreadID.xy, gInputScene.Length.xy - 1)] + gOutput[min(dispatchThreadID.xy, gOutput.Length.xy - 1)];

}