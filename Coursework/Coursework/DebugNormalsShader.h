#pragma once

#ifndef _NORMALDEBUGSHADER_H_
#define _NORMALDEBUGSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;


class DebugNormalsShader : public BaseShader
{
private:
	

public:

	DebugNormalsShader(ID3D11Device* device, HWND hwnd);
	~DebugNormalsShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	void initShader(const wchar_t* vsFilename, const wchar_t* gsFilename, const wchar_t* psFilename);

private:
	ID3D11Buffer* matrixBuffer;
};

#endif