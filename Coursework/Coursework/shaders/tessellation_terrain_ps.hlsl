#define N_LIGHTS 4

Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture[N_LIGHTS * 6] : register(t1);

SamplerState diffuseSampler : register(s0);
SamplerState shadowSampler : register(s1);

cbuffer LightBuffer : register(b0)
{
    float4 ambient[N_LIGHTS];
    float4 diffuse[N_LIGHTS];
    float4 direction[N_LIGHTS];
    int4 light_type[N_LIGHTS];
    float4 lightPosition[N_LIGHTS];
    float4 lightIntensity[N_LIGHTS];
    float4 attenuation_factors[N_LIGHTS];
    float4 spot_falloff[N_LIGHTS];
    float4 spot_angle[N_LIGHTS];
    float4 specular_power[N_LIGHTS];
    float4 specular_colour[N_LIGHTS];
    float4 shadow_bias[N_LIGHTS]; //shadow acne 0.f ; shadow seperation above 0.005f (like 0.05f)
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos[N_LIGHTS * 6] : TEXCOORD3;
};

float4 calculateSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4 specularColour, float specularPower)
{
    float3 halfway = normalize(lightDirection + viewVector);
    float specularIntensity = pow(max(dot(normal, halfway), 0.f), specularPower);
    return saturate(specularColour * specularIntensity);
}

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse, float diff)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = diff * saturate(diffuse * intensity);
    return colour;
}

// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    return !(uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f);
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;
	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    return lightDepthValue > depthValue;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 main(InputType input) : SV_TARGET
{
    float4 colour = float4(0.f, 0.f, 0.f, 0.f);
    float4 specular_new_colour = float4(0.f, 0.f, 0.f, 0.f);
    float4 texture_colour = shaderTexture.Sample(diffuseSampler, input.tex);
    float4 ambient_avg = float4(0.f, 0.f, 0.f, 0.f);
    int ambient_count = 0;
	
    for (int i = 0, mapID = 0; i < N_LIGHTS; ++i, mapID += 6)
    {
		//Calculate the distance between the pixel's normal and the light's position (used for pointlight and spotlight only)
        float3 lightVector = lightPosition[i].xyz - input.worldPosition;
		
		//Before normalising the light vector to a unit vector, use the length to calculate attenuation
        float lightDistance = length(lightVector);
        float attenuation = 1.f / (attenuation_factors[i].x + attenuation_factors[i].y * lightDistance + attenuation_factors[i].z * lightDistance * lightDistance);

        lightVector = normalize(lightVector);

		//Calculate the cosAngle between the light's center direction and the vector to the pixel's position
        float dotVal = dot(lightVector, normalize(-direction[i].xyz));

		//Trick by Erin to make the spotlight look nicer
        float diff = dotVal - cos(3.1415f * spot_angle[i].x / 180.f); // if( dotVal > cosSpotAngle ) then (dotVal - cosSotAngle) will be positive TODO: gui cosSpotAngle
        diff = max(0.f, diff); // if negative, not in light range (therefore set it to 0 for no lighting!)
        diff *= spot_falloff[i].x; // lets get most fractional vals
        diff = min(1.f, diff); // now all diff will be in [0,1] range. 0 is out of light, 1 is in light. 0 < x < 1 is in soft fall off

        // Calculate the projected texture coordinates.
        float2 pTexCoord = getProjectiveCoords(input.lightViewPos[mapID]);
	
		//Check if coord is in bound and check if it is in shadow, if neither light the pixel
        float validDepthData = (float) hasDepthData(pTexCoord);
        float pixelInShadow = (float) !isInShadow(depthMapTexture[mapID], pTexCoord, input.lightViewPos[mapID], shadow_bias[i].x);

		//Directional
        colour += (light_type[i].x == 1) * validDepthData * pixelInShadow * calculateLighting(-direction[i].xyz, input.normal, diffuse[i], lightIntensity[i].x);
		//Spot
        colour += (light_type[i].x == 3) * validDepthData * pixelInShadow * calculateLighting(-direction[i].xyz, input.normal, diffuse[i], diff * lightIntensity[i].x) * attenuation;
		//Point
        for (int j = 0; j < 6; ++j)
        {
			//NEED TO UPDATE THE TEXCOORD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//Otherwise hasDepthData() will be wrong, only based on the first index (mapID)
            pTexCoord = getProjectiveCoords(input.lightViewPos[mapID + j]);

            float point_validDepthData = (float) hasDepthData(pTexCoord);
            float point_pixelInShadow = (float) !isInShadow(depthMapTexture[mapID + j], pTexCoord, input.lightViewPos[mapID + j], shadow_bias[i].x);

            colour += (light_type[i].x == 2) * point_validDepthData * point_pixelInShadow * calculateLighting(lightVector, input.normal, diffuse[i], lightIntensity[i].x) * attenuation;

			//Calculate specular value (zero'd if light type is OFF, or if the specular power is 0)
			//The following specular is only calculated for point lights
            specular_new_colour += (specular_power[i].x != 0.f) * (light_type[i].x == 2) * point_validDepthData * point_pixelInShadow * //
				((light_type[i].x == 3) * diff + (light_type[i].x != 3)) * //Make sure the specular doesnt happen outside of the spotlight area
				(light_type[i].x != 0) * ((light_type[i].x != 1) * attenuation + (light_type[i] == 1)) * //This second calculation either applies attenuation only on point and spot lights's specular
				calculateSpecular((light_type[i].x == 1) * -direction[i].xyz + (light_type[i].x != 1) * lightVector, input.normal, input.viewVector, specular_colour[i], specular_power[i].x);
        }
		
		//Calculate specular value (zero'd if light type is OFF, or if the specular power is 0)
		//The following specular is only calculated for directional and spot lights
        specular_new_colour += (specular_power[i].x != 0.f) * (light_type[i].x != 2) * validDepthData * pixelInShadow * //
				((light_type[i].x == 3) * diff + (light_type[i].x != 3)) * //Make sure the specular doesnt happen outside of the spotlight area
				(light_type[i].x != 0) * ((light_type[i].x != 1) * attenuation + (light_type[i] == 1)) * //This second calculation either applies attenuation only on point and spot lights's specular
				calculateSpecular((light_type[i].x == 1) * -direction[i].xyz + (light_type[i].x != 1) * lightVector, input.normal, input.viewVector, specular_colour[i], specular_power[i].x);

        ambient_avg += (light_type[i].x != 0) * ambient[i];
        ambient_count += (light_type[i].x != 0);
    }
    
    ambient_avg /= (float) ambient_count;
    colour = saturate(colour + ambient_avg);
    return colour * texture_colour + specular_new_colour;
}