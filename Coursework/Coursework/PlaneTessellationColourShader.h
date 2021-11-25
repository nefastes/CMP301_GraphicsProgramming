#pragma once
#include "DXF.h"
using namespace std;
using namespace DirectX;
class PlaneTessellationColourShader : public BaseShader
{
	//Additional buffers
	struct CameraBufferType
	{
		XMFLOAT4 cameraPosition;
		XMFLOAT2 minMaxLOD;
		XMFLOAT2 minMaxDistance;
	};

public:

	PlaneTessellationColourShader(ID3D11Device* device, HWND hwnd);
	~PlaneTessellationColourShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view,
		const XMMATRIX& projection, XMFLOAT4& tess_pos, XMFLOAT2& minMaxLOD, XMFLOAT2& minMaxDistance);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* cameraBuffer;
};

