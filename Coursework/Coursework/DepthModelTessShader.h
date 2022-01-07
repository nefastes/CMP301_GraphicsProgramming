// Colour shader.h
// Simple shader example.
#pragma once

#include "DXF.h"
#include "TModel.h"

using namespace std;
using namespace DirectX;


class DepthModelTessShader : public BaseShader
{

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct HsSettingsBufferType
	{
		XMFLOAT4 tessellationCenterPosition;
		XMFLOAT2 minMaxLOD;
		XMFLOAT2 minMaxDistance;
		XMFLOAT2 tessellation_factors;
		XMFLOAT2 padding;
	};

	struct DsSettingsBufferType
	{
		float height_amplitude;
		XMFLOAT3 padding;
	};

public:

	DepthModelTessShader(ID3D11Device* device, HWND hwnd);
	~DepthModelTessShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		TModel* model, Camera* camera);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* HsSettingsBuffer;
	ID3D11Buffer* DsSettingsBuffer;
};
