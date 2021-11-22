// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#define N_LIGHTS 4

#include "DXF.h"
#include "ShadowMap.h"
#include <memory>

using namespace std;
using namespace DirectX;


class ShadowShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[N_LIGHTS * 6];
		XMMATRIX lightProjection[N_LIGHTS];
	};

	struct LightBufferType
	{
		XMFLOAT4 ambient[N_LIGHTS];
		XMFLOAT4 diffuse[N_LIGHTS];
		XMFLOAT4 direction[N_LIGHTS];			//cause of chunks, last float is padding
		XMINT4 light_type[N_LIGHTS];			//cause of chunks the light type value is repeated on all slots
		XMFLOAT4 lightPosition[N_LIGHTS];
		XMFLOAT4 lightIntensity[N_LIGHTS];		//value repeated on all components
		XMFLOAT4 attenuation_factors[N_LIGHTS];	//cause of chunks, last float is padding
		XMFLOAT4 spot_falloff[N_LIGHTS];		//value repeated on all components
		XMFLOAT4 spot_angle[N_LIGHTS];			//value repeated on all components
		XMFLOAT4 specular_power[N_LIGHTS];		//value repeated on all components
		XMFLOAT4 specular_colour[N_LIGHTS];
		XMFLOAT4 shadow_bias[N_LIGHTS];			//value repeated on all components
	};

	struct CameraBufferType
	{
		XMFLOAT3 position;
		float padding;
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection,
		ID3D11ShaderResourceView* texture, std::unique_ptr<ShadowMap>* maps, std::unique_ptr<Light>* light, Camera* camera);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
};

#endif