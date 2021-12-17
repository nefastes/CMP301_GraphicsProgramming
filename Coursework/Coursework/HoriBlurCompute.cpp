#include "HoriBlurCompute.h"

HoriBlurCompute::HoriBlurCompute(ID3D11Device* device, HWND hwnd, int screen_width, int screen_height) : BaseShader(device, hwnd),
output_texture(nullptr), srv(nullptr), uav(nullptr)
{
	initShader(L"blur_horizontal_cs.cso", screen_width, screen_height);
}

HoriBlurCompute::~HoriBlurCompute()
{
}

void HoriBlurCompute::setShaderParameters(ID3D11DeviceContext* device_context, ID3D11ShaderResourceView* buffer_to_blur)
{
	//Send the buffer to be blurred to the compute shader
	device_context->CSSetShaderResources(0, 1, &buffer_to_blur);
	//Send the UAV to the compute shader so we can write the output to it
	device_context->CSSetUnorderedAccessViews(0, 1, &uav, 0);
}

void HoriBlurCompute::unbind(ID3D11DeviceContext* device_context)
{
	//This function clears the memory that was initialised and used by the compute shader
	//It also disables the compute shader stage on this instance

	//Unbind buffer from compute shader
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	device_context->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	device_context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	device_context->CSSetShader(nullptr, nullptr, 0);
}

void HoriBlurCompute::initShader(const wchar_t* filename, int screen_width, int screen_height)
{
	//Initialise the compute shader stage
	loadComputeShader(filename);

	//Initialise texture buffer and views
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = screen_width;
	textureDesc.Height = screen_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	renderer->CreateTexture2D(&textureDesc, 0, &output_texture);

	//Make sure the texture buffer was successfully initialised
	if (output_texture)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
		ZeroMemory(&descUAV, sizeof(descUAV));
		descUAV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; ;// DXGI_FORMAT_UNKNOWN;
		descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		descUAV.Texture2D.MipSlice = 0;
		renderer->CreateUnorderedAccessView(output_texture, &descUAV, &uav);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		renderer->CreateShaderResourceView(output_texture, &srvDesc, &srv);
	}
	else
	{
		MessageBox(NULL, L"Compute Shader Error: The output texture failed to initialise. Aborting..", L"ERROR", MB_OK);
		exit(1);
	}
}
