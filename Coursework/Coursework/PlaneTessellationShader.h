#pragma once
#include "DXF.h"

#define N_LIGHTS 4

using namespace std;
using namespace DirectX;

class PlaneTessellationShader : public BaseShader
{
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[N_LIGHTS * 6];
		XMMATRIX lightProjection[N_LIGHTS];
	};

	//Additional buffers
	struct LightBufferType
	{
		XMFLOAT4 ambient[N_LIGHTS];
		XMFLOAT4 diffuse[N_LIGHTS];
		XMFLOAT4 direction[N_LIGHTS];			//cause of chunks, last float is padding
		XMFLOAT4 lightPosition[N_LIGHTS];
		XMFLOAT4 attenuation_factors[N_LIGHTS];	//cause of chunks, last float is padding
		XMFLOAT4 specular_colour[N_LIGHTS];
		XMFLOAT4 type_intensity_specularPower_shadowBias[N_LIGHTS];
		XMFLOAT4 falloff_spotAngle_renderNormals_padding[N_LIGHTS];
	};

	struct SettingsBufferType
	{
		XMFLOAT4 tessellationCenterPosition;
		XMFLOAT2 minMaxLOD;
		XMFLOAT2 minMaxDistance;
	};

public:

	PlaneTessellationShader(ID3D11Device* device, HWND hwnd);
	~PlaneTessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap,
		XMFLOAT2& minMaxLOD, XMFLOAT2& minMaxDistance, std::unique_ptr<ShadowMap>* maps, std::unique_ptr<Light>* light, Camera* camera, bool render_normals);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* settingsBuffer;
};

