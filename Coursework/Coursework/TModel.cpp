#include "TModel.h"

TModel::TModel(ID3D11Device* device, const std::string& file) : AModel(device, file), tessellation_factors_(XMFLOAT2(1.f, 1.f)),
height_amplitude_(0.f), use_normal_map_(false), texture_diffuse_(nullptr), texture_normalMap_(nullptr), texture_heightMap_(nullptr)
{
}

TModel::~TModel()
{
}
