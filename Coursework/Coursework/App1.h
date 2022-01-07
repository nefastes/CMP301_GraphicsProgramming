// Application.h
#ifndef _APP1_H
#define _APP1_H
#define N_LIGHTS 4

// Includes
#include "DXF.h"

#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "DepthTerrainTessShader.h"
#include "DepthModelTessShader.h"
#include "LightDebugShader.h"
#include "DebugNormalsShader.h"
#include "PlaneTessellationShader.h"
#include "ModelTessellationShader.h"
#include "BloomThresholdCompute.h"
#include "HoriBlurCompute.h"
#include "VertBlurCompute.h"
#include "BloomCombineCompute.h"
#include "BloomCompute.h"
#include "GrassShader.h"

#include "TerrainMesh.h"

#include <memory>
#include <array>

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass();
	void firstPass();
	void bloomPass();
	void finalPass();
	void gui();

private:
	void renderObjects(const XMMATRIX& view, const XMMATRIX& proj, std::unique_ptr<ShadowMap>* maps, bool renderDepth);
	void renderTessellatedModel(AModel* model, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj, const XMFLOAT3& scale,
		std::unique_ptr<ShadowMap>* maps, ID3D11ShaderResourceView* texture_diffuse, ID3D11ShaderResourceView* texture_normal,
		ID3D11ShaderResourceView* texture_height, bool renderDepth);
	void renderTessellatedTerrain(TerrainMesh* terrain, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj,
		std::unique_ptr<ShadowMap>* maps, ID3D11ShaderResourceView* texture_diffuse, ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* normalmap, bool renderDepth);

	//Shaders
	std::unique_ptr<ShadowShader> light_shader;
	std::unique_ptr<DepthShader> depth_shader;
	std::unique_ptr<DepthTerrainTessShader> depth_tess_terrain_shader;
	std::unique_ptr<DepthModelTessShader> depth_tess_model_shader;
	std::unique_ptr<LightDebugShader> light_debug_shader;
	std::unique_ptr<PlaneTessellationShader> terrain_tess_shader;
	std::unique_ptr<ModelTessellationShader> model_tess_shader;
	std::unique_ptr<TextureShader> texture_shader;
	std::unique_ptr<DebugNormalsShader> debug_normals_shader;
	std::unique_ptr<BloomThresholdCompute> bloom_threshold_compute;
	std::unique_ptr<HoriBlurCompute> horizontal_blur_compute;
	std::unique_ptr<VertBlurCompute> vertical_blur_compute;
	std::unique_ptr<BloomCombineCompute> bloom_combine_compute;
	std::unique_ptr<BloomCompute> bloom_compute;
	std::unique_ptr<GrassShader> grass_shader;

	//Shadowmaps
	std::array<std::unique_ptr<ShadowMap>, N_LIGHTS * 6> shadowmap;	//Create 6 * N_LIGHTS if all of them are point lights

	//Bloom
	std::unique_ptr<RenderTexture> bloom_scene_render_target;

	//Models
	std::unique_ptr<AModel> model_mei;
	std::unique_ptr<AModel> model_totoro;
	std::unique_ptr<AModel> model_rock;
	std::unique_ptr<AModel> model_bench;
	std::unique_ptr<AModel> model_lamp;
	std::unique_ptr<AModel> model_pillar;

	//Meshes
	std::unique_ptr<OrthoMesh> orthomesh_debug_shadow_maps;
	std::unique_ptr<OrthoMesh> orthomesh_display;
	std::unique_ptr<SphereMesh> mesh_light_debug_sphere;
	std::unique_ptr<TerrainMesh> mesh_terrain;
	std::unique_ptr<TerrainMesh> mesh_floor;

	//General guis
	bool gui_render_normals;

	//up to MAX_N_LIGHTS lights, gui for the light properties 
	bool gui_render_light_sphere;
	std::array<std::unique_ptr<Light>, N_LIGHTS> light;
	std::array<bool, N_LIGHTS> gui_light_drop_menu_activate;
	std::array<XMFLOAT2, N_LIGHTS> gui_light_scene_dimensions;
	std::array<XMFLOAT3, N_LIGHTS> gui_light_direction;
	std::array<XMFLOAT3, N_LIGHTS> gui_light_position;
	std::array<XMFLOAT2, N_LIGHTS> gui_light_frustum;
	std::array<XMFLOAT4, N_LIGHTS> gui_light_diffuse_colour;
	std::array<XMFLOAT4, N_LIGHTS> gui_light_ambient_colour;
	int gui_shadow_map_display_index;	//which shadow map to be displayed in the orthomesh
	std::array<int, N_LIGHTS> gui_light_type;
	std::array<float, N_LIGHTS> gui_light_intensity;
	std::array<XMFLOAT3, N_LIGHTS> gui_light_attenuation_factors;
	std::array<float, N_LIGHTS> gui_light_spot_falloff;
	std::array<float, N_LIGHTS> gui_light_spot_angle;
	std::array<float, N_LIGHTS> gui_light_specular_power;
	std::array<bool, N_LIGHTS> gui_light_specular_sameAsDiffuse;	//are we using the same specular colour as the diffuse colour?
	std::array<XMFLOAT4, N_LIGHTS> gui_light_specular_colour;
	std::array<float, N_LIGHTS> gui_light_shadow_bias;

	//Gui tessellation
	XMFLOAT2 gui_model_tessellation_factors;
	float gui_model_height_amplitude;

	//Gui Bloom
	float gui_bloom_threshold;
	int gui_bloom_blur_iterations;
};

#endif