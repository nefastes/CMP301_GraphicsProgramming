// Colour shader.h
// Simple shader example.
#pragma once

#define N_LIGHTS 4

#include "DXF.h"

using namespace std;
using namespace DirectX;


class DepthTessShader : public BaseShader
{

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct SettingsBufferType
	{
		XMFLOAT4 tessellationCenterPosition;
		XMFLOAT2 minMaxLOD;
		XMFLOAT2 minMaxDistance;
	};

public:

	DepthTessShader(ID3D11Device* device, HWND hwnd);
	~DepthTessShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		ID3D11ShaderResourceView* heightMap, XMFLOAT2& minMaxLOD, XMFLOAT2& minMaxDistance, Camera* camera);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* settingsBuffer;
};
