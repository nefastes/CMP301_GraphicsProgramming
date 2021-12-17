#pragma once
#include "BaseShader.h"

class BloomThresholdCompute : public BaseShader
{
private:
	struct SettingsBufferType
	{
		float treshold_value;
		XMFLOAT3 padding;
	};

public:
	BloomThresholdCompute(ID3D11Device* device, HWND hwnd, int screen_width, int screen_height);
	~BloomThresholdCompute();

	void setShaderParameters(ID3D11DeviceContext* device_context, ID3D11ShaderResourceView* buffer_to_threshold, float treshold_value);
	ID3D11ShaderResourceView* getShaderResourceView() { return srv; }
	void unbind(ID3D11DeviceContext* device_context);

private:
	void initShader(const wchar_t* filename, const wchar_t* UNUSED_CAUSE_OF_OVERRIDE) {}
	void initShader(const wchar_t* filename, int screen_width, int sreen_height);

	//Resource views (all initialised in initShader)
	ID3D11Texture2D* output_texture;		//The texture data that will be created
	ID3D11ShaderResourceView* srv;			//The texture data packed as a SRV, so it can be used by other shaders
	ID3D11UnorderedAccessView* uav;			//The texture data packed in a writable view, so it can be modified by the compute shader

	//Additional buffer
	ID3D11Buffer* settingsBuffer;
};

