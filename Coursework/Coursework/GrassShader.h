#pragma once

#include "DXF.h"
#include "TerrainMesh.h"

using namespace std;
using namespace DirectX;


class GrassShader : public BaseShader
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
	};

	struct DsSettingsBufferType
	{
		float height_amplitude;
		XMFLOAT2 texture_scale;
		float padding;
	};

public:

	GrassShader(ID3D11Device* device, HWND hwnd);
	~GrassShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, TerrainMesh* terrain, Camera* camera);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* HsSettingsBuffer;
	ID3D11Buffer* DsSettingsBuffer;
};
