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
	struct SettingsBufferType
	{
		XMFLOAT4 tessellationCenterPosition;
		XMFLOAT2 minMaxLOD;
		XMFLOAT2 minMaxDistance;
	};

public:

	PlaneTessellationShader(ID3D11Device* device, HWND hwnd);
	~PlaneTessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture,
		XMFLOAT2& minMaxLOD, XMFLOAT2& minMaxDistance, std::unique_ptr<ShadowMap>* maps, std::unique_ptr<Light>* light, Camera* camera);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* settingsBuffer;
};

