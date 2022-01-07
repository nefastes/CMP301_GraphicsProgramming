#pragma once
#include "AModel.h"

class TModel : public AModel
{
public:
	TModel(ID3D11Device* device, const std::string& file);
	~TModel();

	XMFLOAT2* getPtrTessellationFactors() { return &tessellation_factors_; }
	float* getPtrHeightAmplitude() { return &height_amplitude_; }
	bool* getPtrNormalMap() { return &use_normal_map_; }

	void setTextureDiffuse(ID3D11ShaderResourceView* t) { texture_diffuse_ = t; }
	void setTextureNormalMap(ID3D11ShaderResourceView* t) { texture_normalMap_ = t; }
	void setTextureHeightMap(ID3D11ShaderResourceView* t) { texture_heightMap_ = t; }
	ID3D11ShaderResourceView* const* getPtrTextureDiffuse() { return &texture_diffuse_; }
	ID3D11ShaderResourceView* const* getPtrTextureNormalMap() { return &texture_normalMap_; }
	ID3D11ShaderResourceView* const* getPtrTextureHeightMap() { return &texture_heightMap_; }

private:
	XMFLOAT2 tessellation_factors_;
	float height_amplitude_;
	bool use_normal_map_;

	//Store the textures in here
	ID3D11ShaderResourceView* texture_diffuse_;
	ID3D11ShaderResourceView* texture_normalMap_;
	ID3D11ShaderResourceView* texture_heightMap_;
};

