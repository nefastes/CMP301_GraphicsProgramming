#include "DepthTessShader.h"

DepthTessShader::DepthTessShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellation_terrain_vs.cso", L"depth_ps.cso");
	loadHullShader(L"tessellation_terrain_fodd_hs.cso");
	loadDomainShader(L"tessellation_terrain_depth_ds.cso");
}

DepthTessShader::~DepthTessShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void DepthTessShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Setup tessellation settings buffer
	D3D11_BUFFER_DESC settingsBufferDesc;
	settingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	settingsBufferDesc.ByteWidth = sizeof(SettingsBufferType);
	settingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	settingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	settingsBufferDesc.MiscFlags = 0;
	settingsBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&settingsBufferDesc, NULL, &settingsBuffer);

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

}

void DepthTessShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
	ID3D11ShaderResourceView* heightMap, XMFLOAT2& minMaxLOD, XMFLOAT2& minMaxDistance, Camera* camera)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	////HULL SHADER BUFFERS AND RESOURCES
	result = deviceContext->Map(settingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	SettingsBufferType* settingsPtr = (SettingsBufferType*)mappedResource.pData;
	XMFLOAT3 cameraPos = camera->getPosition();
	settingsPtr->tessellationCenterPosition = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 1.f);
	settingsPtr->minMaxLOD = minMaxLOD;
	settingsPtr->minMaxDistance = minMaxDistance;
	deviceContext->Unmap(settingsBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &settingsBuffer);

	////DOMAIN SHADER BUFFERS AND RESOURCES
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->DSSetSamplers(0, 1, &sampleState);
}
