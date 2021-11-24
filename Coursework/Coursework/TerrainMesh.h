#pragma once
#include "BaseMesh.h"
using namespace DirectX;
class TerrainMesh : public BaseMesh
{
public:
	TerrainMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~TerrainMesh();

	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST) override;

	int clamp(int n, int lower, int upper);

protected:
	void initBuffers(ID3D11Device* device);
};

