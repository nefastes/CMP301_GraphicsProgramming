#include "PlaneTessellationShader.h"

PlaneTessellationShader::PlaneTessellationShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellation_terrain_vs.cso", L"tessellation_terrain_fodd_hs.cso", L"tessellation_terrain_ds.cso", L"tessellation_terrain_ps.cso");
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
	ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightMap,
	XMFLOAT2& minMaxLOD, XMFLOAT2& minMaxDistance, std::unique_ptr<ShadowMap>* maps, std::unique_ptr<Light>* light, Camera* camera, bool render_normals)
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
	deviceContext->DSSetShaderResources(0, 1, &heightMap);
	deviceContext->DSSetSamplers(0, 1, &sampleState);

	////PIXEL SHADER BUFFERS AND RESOURCES
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	LightBufferType* lightPtr = (LightBufferType*)mappedResource.pData;
	for (int i = 0; i < N_LIGHTS; ++i)
	{
		lightPtr->ambient[i] = light[i]->getAmbientColour();
		lightPtr->diffuse[i] = light[i]->getDiffuseColour();
		XMFLOAT3 direction = light[i]->getDirection();
		lightPtr->direction[i] = XMFLOAT4(direction.x, direction.y, direction.z, 1.f);
		XMFLOAT3 position = light[i]->getPosition();
		lightPtr->lightPosition[i] = XMFLOAT4(position.x, position.y, position.z, 1.f);
		XMFLOAT3 attenuatio_factors = light[i]->getAttenuationFactors();
		lightPtr->attenuation_factors[i] = XMFLOAT4(attenuatio_factors.x, attenuatio_factors.y, attenuatio_factors.z, 1.f);
		lightPtr->specular_colour[i] = light[i]->getSpecularColour();

		//Better data compression
		//Compress type, intensity, specular_power and shadow_bias into a unique float4
		lightPtr->type_intensity_specularPower_shadowBias[i] = XMFLOAT4((float)light[i]->getType(), light[i]->getIntensity(), light[i]->getSpecularPower(), light[i]->getShadowBias());
		//Compress falloff, spotangle, renderNormals and a padding into a unique float4
		lightPtr->falloff_spotAngle_renderNormals_padding[i] = XMFLOAT4(light[i]->getSpotFalloff(), light[i]->getSpotAngle(), (float)render_normals, 1.f);
	}
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &heightMap);
	for (int i = 0; i < N_LIGHTS * 6; ++i)
	{
		ID3D11ShaderResourceView* depthMap = maps[i]->getDepthMapSRV();
		deviceContext->PSSetShaderResources(i + 2, 1, &depthMap);
	}
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleStateShadow);
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

	// Setup light buffer
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

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
	// Sampler for shadow map sampling.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&samplerDesc, &sampleStateShadow);
}

void PlaneTessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}
