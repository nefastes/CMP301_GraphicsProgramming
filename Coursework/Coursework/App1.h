// Application.h
#ifndef _APP1_H
#define _APP1_H
#define N_LIGHTS 4

// Includes
#include "DXF.h"

#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "LightDebugShader.h"
#include "PlaneTessellationShader.h"

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
	void finalPass();
	void gui();

private:
	void renderObjects(const XMMATRIX& view, const XMMATRIX& proj, std::unique_ptr<ShadowMap>* map, bool renderDepth);

	//Shaders
	std::unique_ptr<ShadowShader> shadow_shader;
	std::unique_ptr<DepthShader> depth_shader;
	std::unique_ptr<LightDebugShader> light_debug_shader;
	std::unique_ptr<PlaneTessellationShader> tess_shader;
	std::unique_ptr<TextureShader> texture_shader;

	//Shadowmaps
	std::array<std::unique_ptr<ShadowMap>, N_LIGHTS * 6> shadowmap;	//Create 6 * N_LIGHTS if all of them are point lights

	//Models
	std::unique_ptr<AModel> model;

	//Meshes
	std::unique_ptr<OrthoMesh> orthomesh;
	std::unique_ptr<PlaneMesh> mesh;
	std::unique_ptr<CubeMesh> cube;
	std::unique_ptr<SphereMesh> sphere;
	std::unique_ptr<SphereMesh> light_debug_sphere;
	std::unique_ptr<TerrainMesh> terrain;
	float objects_roation_angle;

	//Simple gui boolean to animate the objects (or not)
	bool gui_animate_objects;

	//up to MAX_N_LIGHTS lights, gui for the light properties 
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
	XMFLOAT2 gui_min_max_LOD;
	XMFLOAT2 gui_min_max_distance;
};

#endif