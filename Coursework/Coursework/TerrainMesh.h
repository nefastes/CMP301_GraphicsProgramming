#pragma once
#include "BaseMesh.h"
using namespace DirectX;
class TerrainMesh : public BaseMesh
{
public:
	TerrainMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~TerrainMesh();

	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST) override;
	XMFLOAT2* getPtrMinMaxLOD() { return &min_max_LOD_; }
	XMFLOAT2* getPtrMinMaxDistance() { return &min_max_distance_; }
	XMFLOAT2* getPtrTextureScale() { return &terrain_texture_scale_; }
	float* getPtrHeightAmplitude() { return &terrain_height_amplitude_; }
	bool* getPtrNormalMap() { return &use_normal_map_; }

protected:
	void initBuffers(ID3D11Device* device);
	XMFLOAT2 min_max_LOD_;
	XMFLOAT2 min_max_distance_;
	XMFLOAT2 terrain_texture_scale_;
	float terrain_height_amplitude_;
	bool use_normal_map_;
};

