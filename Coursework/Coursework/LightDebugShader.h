#ifndef _LIGHTDEBUGSHADER_H_
#define _LIGHTDEBUGSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;


class LightDebugShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightBufferType
	{
		XMFLOAT4 colour;
	};

public:

	LightDebugShader(ID3D11Device* device, HWND hwnd);
	~LightDebugShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Light* light);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
};

#endif