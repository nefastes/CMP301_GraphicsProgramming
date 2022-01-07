#include "DepthModelTessShader.h"

DepthModelTessShader::DepthModelTessShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellation_model_vs.cso", L"depth_ps.cso");
	loadHullShader(L"tessellation_model_fodd_hs.cso");
	loadDomainShader(L"tessellation_model_depth_ds.cso");
}

DepthModelTessShader::~DepthModelTessShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	if (HsSettingsBuffer)
	{
		HsSettingsBuffer->Release();
		HsSettingsBuffer = 0;
	}
	if (DsSettingsBuffer)
	{
		DsSettingsBuffer->Release();
		DsSettingsBuffer = 0;
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

void DepthModelTessShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
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

	// Setup tessellation HS settings buffer
	D3D11_BUFFER_DESC HsSettingsBufferDesc;
	HsSettingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	HsSettingsBufferDesc.ByteWidth = sizeof(HsSettingsBufferType);
	HsSettingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HsSettingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HsSettingsBufferDesc.MiscFlags = 0;
	HsSettingsBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&HsSettingsBufferDesc, NULL, &HsSettingsBuffer);

	// Setup tessellation DS settings buffer
	D3D11_BUFFER_DESC DsSettingsBufferDesc;
	DsSettingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	DsSettingsBufferDesc.ByteWidth = sizeof(DsSettingsBufferType);
	DsSettingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	DsSettingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DsSettingsBufferDesc.MiscFlags = 0;
	DsSettingsBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&DsSettingsBufferDesc, NULL, &DsSettingsBuffer);

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

void DepthModelTessShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
	TModel* model, Camera* camera)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	////HULL SHADER BUFFERS AND RESOURCES
	result = deviceContext->Map(HsSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	HsSettingsBufferType* HsSettingsPtr = (HsSettingsBufferType*)mappedResource.pData;
	XMFLOAT3 cameraPos = camera->getPosition();
	HsSettingsPtr->tessellationCenterPosition = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 1.f);
	HsSettingsPtr->minMaxLOD = XMFLOAT2(0.f, 0.f);
	HsSettingsPtr->minMaxDistance = XMFLOAT2(0.f, 0.f);
	HsSettingsPtr->tessellation_factors = *model->getPtrTessellationFactors();
	HsSettingsPtr->padding = XMFLOAT2(0.f, 0.f);
	deviceContext->Unmap(HsSettingsBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &HsSettingsBuffer);

	////DOMAIN SHADER BUFFERS AND RESOURCES
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->DSSetShaderResources(0, 1, model->getPtrTextureHeightMap());
	//TODO: ADD NORMAL MAP AND STUFF WHEN THE DISPLACEMENT ON THE ROCK IS SUCESSFULL
	deviceContext->DSSetSamplers(0, 1, &sampleState);
	//Settings buffer
	result = deviceContext->Map(DsSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DsSettingsBufferType* DsSettingsPtr = (DsSettingsBufferType*)mappedResource.pData;
	DsSettingsPtr->height_amplitude = *model->getPtrHeightAmplitude();
	DsSettingsPtr->padding = XMFLOAT3(0.f, 0.f, 0.f);
	deviceContext->Unmap(DsSettingsBuffer, 0);
	deviceContext->DSSetConstantBuffers(1, 1, &DsSettingsBuffer);
}
