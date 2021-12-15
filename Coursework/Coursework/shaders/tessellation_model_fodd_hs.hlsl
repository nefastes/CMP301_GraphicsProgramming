// Tessellation Hull Shader
// Prepares control points for tessellation
cbuffer SettingsBuffer : register(b0)
{
	float4 tessellationCenterPosition;
	float2 minMaxLOD;
	float2 minMaxDistance;
}

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct OutputType
{
	float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 cameraPosition : TEXCOORD1;
};

float3 ComputePatchMidPoint(float3 p1, float3 p2, float3 p3)
{
    return (p1 + p2 + p3) / 3.f;
}

float ComputeScaledDistance(float3 from, float3 to)
{
    // Compute the raw distance from the camera to the midpoint of this patch
    float d = distance( from, to );
    // Scale this to be 0.0 (at the min dist) and 1.0 (at the max dist)
    // Need to saturate it otherwise when the quads disappear when non-tessellated
	return saturate((d - minMaxDistance.x) / (minMaxDistance.y - minMaxDistance.x));
}

float ComputePatchLOD(float3 midPoint)
{
    // Compute the scaled distance
	float d = ComputeScaledDistance(tessellationCenterPosition.xyz, midPoint);
    // Transform this 0.0-1.0 distance scale into the desired LOD�s
    // note: invert the distance so that close = high detail, far = low detail
    return lerp( minMaxLOD.x, minMaxLOD.y, 1.0f - d );
}

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;

	//float3 midPoints[] =
	//{
	//    ComputePatchMidPoint(inputPatch[0].position, inputPatch[1].position, inputPatch[2].position), // main triangle
	//	ComputePatchMidPoint(inputPatch[1].position, inputPatch[2].position, inputPatch[3].position),
	//	ComputePatchMidPoint(inputPatch[0].position, inputPatch[1].position, inputPatch[4].position),
	//	ComputePatchMidPoint(inputPatch[0].position, inputPatch[2].position, inputPatch[5].position)
	//};

	//float dist[] =
	//{
	//	ComputePatchLOD(midPoints[0]),  // main triangle
	//	ComputePatchLOD(midPoints[1]),
	//	ComputePatchLOD(midPoints[2]),
	//	ComputePatchLOD(midPoints[3])
	//};
    
 //   //This patch always has an interior matching the patch LOD
	//output.inside = dist[0];
    
 //   //For the edges its more complex as we have to match the neighboring patches.
 //   //The rule in this case is:
 //   //
 //   // - If the nieghbor patch is of a lower LOD we pick that LOD as the edge for this patch.
 //   //
 //   // - If the neighbor patch is a heigher LOD then we stick with our LOD and expect them to blend down towards us
	//output.edges[0] = min(dist[0], dist[3]);
	//output.edges[1] = min(dist[0], dist[2]);
	//output.edges[2] = min(dist[0], dist[1]);
	
	output.inside = 30;
	output.edges[0] = output.edges[1] = output.edges[2] = 10;
    
    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;

    // Set the input tex as the output tex.
    output.tex = patch[pointId].tex;
    
    // Set the input normal as the output normal.
	output.normal = patch[pointId].normal;
    
    //Send the camera position to the Domain shader
	output.cameraPosition = tessellationCenterPosition.xyz;

    return output;
}