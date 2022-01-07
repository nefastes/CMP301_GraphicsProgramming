// Vertical compute blurr, based on Frank Luna's example.

// Blur weightings
cbuffer cbSettings
{
    static float gWeights[11] =
    {
		.05f, .0675f, .075f, .0875f, .1f, .24f, .1f, .0875f, .075f, .0675f, .05f
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 5;   //For a total of 5 + 5 + 1 = 11 samples
};

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 256
#define CacheSize (N + 2*gBlurRadius)	// + 2*gBlurRadius so that borders have something to blur
groupshared float4 gCache[CacheSize];

[numthreads(1, N, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

    if (groupThreadID.y < gBlurRadius)
    {
		// If the current thread is one on the edge of the array (i.e. if substracting gBlurRadius to it would end up out of bounds of the threadgroup),
		// then cache as many pixel upwards as possible and clamp out of bound samples that occur at image borders.
        int y = max(dispatchThreadID.y - gBlurRadius, 0);
        gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y >= N - gBlurRadius)
    {
		// If the current thread is one on the edge of the array (i.e. if adding gBlurRadius to it would end up out of bounds of the threadgroup),
		// then cache as many pixel downwards as possible and clamp out of bound samples that occur at image borders.
        int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y - 1);
        gCache[groupThreadID.y + 2 * gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
    }

    // Cache the current pixel
	// The min() ensures that the thread caches an input within bounds of the texture
    gCache[groupThreadID.y + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];


	// Wait for all threads to finish.
    GroupMemoryBarrierWithGroupSync();

	// Now blur each pixel.
    float4 blurColor = float4(0, 0, 0, 0);
    //Unroll if the loop will execute a non-variable amount of times (the compiler optimises the condition by pasting the looping code one after another
	[unroll] for(int i = -gBlurRadius; i <= gBlurRadius; ++i) blurColor += gWeights[i + gBlurRadius] * gCache[groupThreadID.y + gBlurRadius + i];

    //Finally, output the new colour to the RWTexture2D
	gOutput[dispatchThreadID.xy] = blurColor;   //dispatchThreadID = pixel location
}