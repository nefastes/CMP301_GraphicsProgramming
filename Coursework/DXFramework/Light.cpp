// Light class
// Holds data that represents a single light source
#include "light.h"

// create view matrix, based on light position and lookat. Used for shadow mapping.
void Light::generateViewMatrix()
{
	// default up vector
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	if (direction.y == 1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0);
	}
	else if (direction.y == -1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0);
	}
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 1.0f);
	XMVECTOR right = XMVector3Cross(dir, up);
	up = XMVector3Cross(right, dir);
	// Create the view matrix from the three vectors.
	viewMatrix = XMMatrixLookAtLH(position, position + dir, up);
}

void Light::generatePointLightViewMatrices()
{
	//Generate the 6 directions for each view matrix
	std::array<XMFLOAT3, 6> point_light_directions = {
		XMFLOAT3(1.f, 0.f, 0.f),
		XMFLOAT3(-1.f, 0.f, 0.f),
		XMFLOAT3(0.f, 1.f, 0.f),
		XMFLOAT3(0.f, -1.f, 0.f),
		XMFLOAT3(0.f, 0.f, 1.f),
		XMFLOAT3(0.f, 0.f, -1.f)
	};

	//Save the current direction (in case it was already assigned)
	XMFLOAT3 saved_direction = direction;
	//Save view matrix for same reasons
	XMMATRIX saved_view = viewMatrix;

	//Generate the matrices
	for (unsigned short i = 0u; i < 6; ++i)
	{
		direction = point_light_directions[i];
		generateViewMatrix();
		point_light_view_matrices[i] = viewMatrix;
	}

	//Restore the direction and view
	direction = saved_direction;
	viewMatrix = saved_view;
}

// Create a projection matrix for the (point) light source. Used in shadow mapping.
void Light::generateProjectionMatrix(float screenNear, float screenFar)
{
	float fieldOfView, screenAspect;

	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)XM_PI / 2.0f;
	screenAspect = 1.0f;

	// Create the projection matrix for the light.
	projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenFar);
}

// Create orthomatrix for (directional) light source. Used in shadow mapping.
void Light::generateOrthoMatrix(float screenWidth, float screenHeight, float near, float far)
{
	orthoMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, near, far);
}

void Light::setAmbientColour(float red, float green, float blue, float alpha)
{
	ambientColour = XMFLOAT4(red, green, blue, alpha);
}

void Light::setDiffuseColour(float red, float green, float blue, float alpha)
{
	diffuseColour = XMFLOAT4(red, green, blue, alpha);
}

void Light::setDirection(float x, float y, float z)
{
	direction = XMFLOAT3(x, y, z);
}

void Light::setSpecularColour(float red, float green, float blue, float alpha)
{
	specularColour = XMFLOAT4(red, green, blue, alpha);
}

void Light::setSpecularPower(float power)
{
	specularPower = power;
}

void Light::setPosition(float x, float y, float z)
{
	position = XMVectorSet(x, y, z, 1.0f);
}

XMFLOAT4 Light::getAmbientColour()
{
	return ambientColour;
}

XMFLOAT4 Light::getDiffuseColour()
{
	return diffuseColour;
}


XMFLOAT3 Light::getDirection()
{
	return direction;
}

XMFLOAT4 Light::getSpecularColour()
{
	return specularColour;
}


float Light::getSpecularPower()
{
	return specularPower;
}

XMFLOAT3 Light::getPosition()
{
	XMFLOAT3 temp(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position));
	return temp;
}

void Light::setLookAt(float x, float y, float z)
{
	lookAt = XMVectorSet(x, y, z, 1.0f);
}

void Light::setType(int t)
{
	type = t;
}

void Light::setIntensity(float i)
{
	intensity = i;
}

void Light::setAttenuationFactors(float c1, float c2, float c3)
{
	attenuation_factors = XMFLOAT3(c1, c2, c3);
}

void Light::setAttenuationFactors(XMFLOAT3& factors)
{
	attenuation_factors = factors;
}

void Light::setSpotFalloff(float f)
{
	spot_falloff = f;
}

void Light::setSpotAngle(float a)
{
	spot_angle = a;
}

void Light::setShadowBias(float b)
{
	shadow_bias = b;
}

XMMATRIX Light::getViewMatrix()
{
	return viewMatrix;
}

XMMATRIX Light::getProjectionMatrix()
{
	return projectionMatrix;
}

XMMATRIX Light::getOrthoMatrix()
{
	return orthoMatrix;
}

XMMATRIX Light::getPointViewMatrix(int index)
{
	return point_light_view_matrices.at(index);
}

int Light::getType()
{
	return type;
}

float Light::getIntensity()
{
	return intensity;
}

XMFLOAT3 Light::getAttenuationFactors()
{
	return attenuation_factors;
}

float Light::getSpotFalloff()
{
	return spot_falloff;
}

float Light::getSpotAngle()
{
	return spot_angle;
}

float Light::getShadowBias()
{
	return shadow_bias;
}
