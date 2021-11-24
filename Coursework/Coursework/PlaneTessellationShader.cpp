#include "PlaneTessellationShader.h"

PlaneTessellationShader::PlaneTessellationShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellation_vs.cso", L"tessellation_terrain_fodd_hs.cso", L"tessellation_terrain_ds.cso", L"tessellation_ps.cso");
}

PlaneTessellationShader::~PlaneTessellationShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void PlaneTessellationShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
	ID3D11ShaderResourceView* texture, XMFLOAT2& minMaxLOD, XMFLOAT2& minMaxDistance, std::unique_ptr<ShadowMap>* maps, std::unique_ptr<Light>* light, Camera* camera)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	for (int i = 0, viewID = 0; i < N_LIGHTS; ++i, viewID += 6)
	{
		int type = light[i]->getType();
		if (type == 2)
			for (int j = 0; j < 6; ++j)
				dataPtr->lightView[viewID + j] = XMMatrixTranspose(light[i]->getPointViewMatrix(j));
		else dataPtr->lightView[viewID] = XMMatrixTranspose(light[i]->getViewMatrix());

		dataPtr->lightProjection[i] = XMMatrixTranspose(type == 1 ? light[i]->getOrthoMatrix() : light[i]->getProjectionMatrix());
	}
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);

	result = deviceContext->Map(settingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	SettingsBufferType* settingsPtr = (SettingsBufferType*)mappedResource.pData;
	XMFLOAT3 cameraPos = camera->getPosition();
	settingsPtr->tessellationCenterPosition = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 1.f);
	settingsPtr->minMaxLOD = minMaxLOD;
	settingsPtr->minMaxDistance = minMaxDistance;
	deviceContext->Unmap(settingsBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &settingsBuffer);
}

void PlaneTessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
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

	D3D11_BUFFER_DESC settingsBufferDesc;
	settingsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	settingsBufferDesc.ByteWidth = sizeof(SettingsBufferType);
	settingsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	settingsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	settingsBufferDesc.MiscFlags = 0;
	settingsBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&settingsBufferDesc, NULL, &settingsBuffer);
}

void PlaneTessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}
