// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1() : gui_shadow_map_display_index(0), gui_render_normals(false),
gui_bloom_enable(true), gui_bloom_threshold(1.5f), gui_bloom_blur_iterations(10), gui_render_light_sphere(true)
{
	for (int i = 0; i < N_LIGHTS; ++i)
	{
		gui_light_drop_menu_activate[i] = false;
		gui_light_scene_dimensions[i] = XMFLOAT2(100.f, 100.f);
		gui_light_direction[i] = XMFLOAT3(0.f, -.7f, .7f);
		gui_light_position[i] = XMFLOAT3(0.f, 0.f, -10.f);
		gui_light_frustum[i] = XMFLOAT2(.1f, 100.f);
		gui_light_diffuse_colour[i] = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		gui_light_ambient_colour[i] = XMFLOAT4(0.f, 0.f, 0.f, 0.f);
		gui_light_type[i] = 0;
		gui_light_intensity[i] = 1.f;
		gui_light_attenuation_factors[i] = XMFLOAT3(.5f, .125f, .01f);
		gui_light_spot_falloff[i] = 64.f;
		gui_light_spot_angle[i] = 30.f;
		gui_light_specular_power[i] = 0.f;
		gui_light_specular_sameAsDiffuse[i] = true;
		gui_light_specular_colour[i] = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		gui_light_shadow_bias[i] = 0.004f;
	}
	//Light 1 custom setup
	gui_light_type[0] = 1;
	gui_light_direction[0] = XMFLOAT3(.25f, -.125f, .7f);
	gui_light_position[0] = XMFLOAT3(-50.f, 30.f, -125.f);
	gui_light_frustum[0] = XMFLOAT2(45.f, 225.f);
	gui_light_diffuse_colour[0] = XMFLOAT4(.995f, .586f, .0f, 1.f);
	gui_light_ambient_colour[0] = XMFLOAT4(.3f, .3f, .3f, 1.f);	//Only the directional light gets an ambient component
	gui_light_specular_power[0] = 40.f;

	//Light 2 curstom setup
	gui_light_type[1] = 2;
	gui_light_position[1] = XMFLOAT3(0.f, 15.f, 0.f);
	gui_light_frustum[1] = XMFLOAT2(5.f, 100.f);
	gui_light_diffuse_colour[1] = XMFLOAT4(1.f, .835f, .377f, 1.f);
	gui_light_attenuation_factors[1] = XMFLOAT3(.5f, .25f, .00125f);
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	//// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	//// Load textures
	textureMgr->loadTexture(L"heightMap", L"res/heightmap3.dds");
	textureMgr->loadTexture(L"noHeightMap", L"res/heightmap0.jpg");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wood", L"res/wood.png");
	textureMgr->loadTexture(L"grass", L"res/grass_2.jpg");
	textureMgr->loadTexture(L"marble_diffuse", L"res/marble_diffuse.jpg");
	textureMgr->loadTexture(L"marble_normal", L"res/marble_normal.jpg");
	textureMgr->loadTexture(L"marble_height", L"res/marble_height.jpg");
	textureMgr->loadTexture(L"model_mei_diffuse", L"res/models/mei/Mei_TEX.png");
	textureMgr->loadTexture(L"model_totoro_diffuse", L"res/models/totoro/Totoro_Map.png");
	textureMgr->loadTexture(L"model_rock_diffuse", L"res/models/rock_model_1/textures/Rock_1_Diffuse.jpg");
	textureMgr->loadTexture(L"model_rock_height", L"res/models/rock_model_1/textures/Rock_1_Glossiness.jpg");
	textureMgr->loadTexture(L"model_rock_normal", L"res/models/rock_model_1/textures/Rock_1_Normal.jpg");
	textureMgr->loadTexture(L"model_bench_diffuse", L"res/models/bench/benchs_diffuse.png");
	textureMgr->loadTexture(L"model_bench_normal", L"res/models/bench/benchs_normal.png");
	textureMgr->loadTexture(L"model_pillar_diffuse", L"res/models/pillar/Textures/Marble_Base_Color.jpg");
	textureMgr->loadTexture(L"model_pillar_normal", L"res/models/pillar/Textures/Marble_Normal_OpenGL.jpg");
	textureMgr->loadTexture(L"model_pillar_height", L"res/models/pillar/Textures/Marble_Height.jpg");

	//// Init shaders
	texture_shader = std::make_unique<TextureShader>(renderer->getDevice(), hwnd);
	depth_shader = std::make_unique<DepthShader>(renderer->getDevice(), hwnd);
	depth_tess_terrain_shader = std::make_unique<DepthTerrainTessShader>(renderer->getDevice(), hwnd);
	depth_tess_model_shader = std::make_unique<DepthModelTessShader>(renderer->getDevice(), hwnd);
	light_shader = std::make_unique<ShadowShader>(renderer->getDevice(), hwnd);
	light_debug_shader = std::make_unique<LightDebugShader>(renderer->getDevice(), hwnd);
	terrain_tess_shader = std::make_unique<PlaneTessellationShader>(renderer->getDevice(), hwnd);
	model_tess_shader = std::make_unique<ModelTessellationShader>(renderer->getDevice(), hwnd);
	debug_normals_shader = std::make_unique<DebugNormalsShader>(renderer->getDevice(), hwnd);
	bloom_threshold_compute = std::make_unique<BloomThresholdCompute>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	horizontal_blur_compute = std::make_unique<HoriBlurCompute>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	vertical_blur_compute = std::make_unique<VertBlurCompute>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	bloom_combine_compute = std::make_unique<BloomCombineCompute>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	bloom_compute = std::make_unique<BloomCompute>(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	grass_shader = std::make_unique<GrassShader>(renderer->getDevice(), hwnd);

	//// Init Shadowmaps
	// Variables for defining shadow map
	int shadowmapWidth = 4096;	//MAX: 16'384
	int shadowmapHeight = 4096;
	//Create as much shadowMaps as there are lights * 6 (for point lights)
	for(int i = 0; i < N_LIGHTS * 6; ++i) shadowmap[i] = std::make_unique<ShadowMap>(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	//// Init lights
	for (int i = 0; i < N_LIGHTS; ++i)
	{
		light[i] = std::make_unique<Light>();
		light[i]->setType(gui_light_type[i]);
		light[i]->setIntensity(gui_light_intensity[i]);
		light[i]->setAmbientColour(gui_light_ambient_colour[i].x, gui_light_ambient_colour[i].y, gui_light_ambient_colour[i].z, gui_light_ambient_colour[i].w);
		light[i]->setDiffuseColour(gui_light_diffuse_colour[i].x, gui_light_diffuse_colour[i].y, gui_light_diffuse_colour[i].z, gui_light_diffuse_colour[i].w);
		light[i]->setDirection(gui_light_direction[i].x, gui_light_direction[i].y, gui_light_direction[i].z);
		light[i]->setPosition(gui_light_position[i].x, gui_light_position[i].y, gui_light_position[i].z);
		light[i]->setAttenuationFactors(gui_light_attenuation_factors[i]);	//TODO: Question Erin about the PS shader behaviour when commenting this
		light[i]->setSpotFalloff(gui_light_spot_falloff[i]);
		light[i]->setSpotAngle(gui_light_spot_angle[i]);
		light[i]->setSpecularPower(gui_light_specular_power[i]);
		light[i]->setSpecularColour(gui_light_diffuse_colour[i].x, gui_light_diffuse_colour[i].y, gui_light_diffuse_colour[i].z, gui_light_diffuse_colour[i].w);	//init spec on diffuse
		light[i]->setShadowBias(gui_light_shadow_bias[i]);
		light[i]->generateOrthoMatrix(gui_light_scene_dimensions[i].x, gui_light_scene_dimensions[i].y, gui_light_frustum[i].x, gui_light_frustum[i].y);
		light[i]->generateProjectionMatrix(gui_light_frustum[i].x, gui_light_frustum[i].y);
		light[i]->generateViewMatrix();
		light[i]->generatePointLightViewMatrices();
	}

	//// Init orthomesh
	orthomesh_debug_shadow_maps = std::make_unique<OrthoMesh>(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 3, screenHeight / 3);
	orthomesh_display = std::make_unique<OrthoMesh>(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);

	//// Init render targets
	bloom_scene_render_target = std::make_unique<RenderTexture>(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	//Init terrains
	mesh_terrain = std::make_unique<TerrainMesh>(renderer->getDevice(), renderer->getDeviceContext(), 10.f, XMFLOAT3(0.f, 2.75f, -15.f));
	*mesh_terrain->getPtrHeightAmplitude() = 1.5f;
	*mesh_terrain->getPtrMinMaxDistance() = XMFLOAT2(5.f, 15.f);
	*mesh_terrain->getPtrMinMaxLOD() = XMFLOAT2(1.f, 15.f);
	*mesh_terrain->getPtrTextureScale() = XMFLOAT2(10.f, 10.f);
	mesh_terrain->setTextureDiffuse(textureMgr->getTexture(L"grass"));
	mesh_terrain->setTextureHeightMap(textureMgr->getTexture(L"heightMap"));
	mesh_floor = std::make_unique<TerrainMesh>(renderer->getDevice(), renderer->getDeviceContext(), 75.f);
	*mesh_floor->getPtrHeightAmplitude() = .45f;
	*mesh_floor->getPtrMinMaxDistance() = XMFLOAT2(50.f, 75.f);
	*mesh_floor->getPtrMinMaxLOD() = XMFLOAT2(1.f, 15.f);
	*mesh_floor->getPtrTextureScale() = XMFLOAT2(10.f, 10.f);
	mesh_floor->setTextureDiffuse(textureMgr->getTexture(L"marble_diffuse"));
	mesh_floor->setTextureNormalMap(textureMgr->getTexture(L"marble_normal"));
	mesh_floor->setTextureHeightMap(textureMgr->getTexture(L"marble_height"));

	//// Init additional objects
	mesh_light_debug_sphere = std::make_unique<SphereMesh>(renderer->getDevice(), renderer->getDeviceContext());

	model_mei = std::make_unique<TModel>(renderer->getDevice(), "res/models/mei/Mei.obj");
	model_mei->setTextureDiffuse(textureMgr->getTexture(L"model_mei_diffuse"));

	model_totoro = std::make_unique<TModel>(renderer->getDevice(), "res/models/totoro/totoro.fbx");
	model_totoro->setTextureDiffuse(textureMgr->getTexture(L"model_totoro_diffuse"));

	model_rock = std::make_unique<TModel>(renderer->getDevice(), "res/models/rock_model_1/rock.obj");
	model_rock->setTextureDiffuse(textureMgr->getTexture(L"model_rock_diffuse"));
	model_rock->setTextureNormalMap(textureMgr->getTexture(L"model_rock_normal"));
	model_rock->setTextureHeightMap(textureMgr->getTexture(L"model_rock_height"));

	model_bench = std::make_unique<TModel>(renderer->getDevice(), "res/models/bench/bench.obj");
	model_bench->setTextureDiffuse(textureMgr->getTexture(L"model_bench_diffuse"));
	model_bench->setTextureNormalMap(textureMgr->getTexture(L"model_bench_normal"));

	model_pillar = std::make_unique<TModel>(renderer->getDevice(), "res/models/pillar/Column_HP.obj");
	model_pillar->setTextureDiffuse(textureMgr->getTexture(L"model_pillar_diffuse"));
	model_pillar->setTextureNormalMap(textureMgr->getTexture(L"model_pillar_normal"));
	model_pillar->setTextureHeightMap(textureMgr->getTexture(L"model_pillar_height"));
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	// Since I'm using smart pointers there isn't the need to delete anything here
}

bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	//other updates

	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{

	// Perform depth pass
	depthPass();
	// Render the scene in a render target for post-processing
	firstPass();
	// Apply a Bloom post-processing effect
	if(gui_bloom_enable) bloomPass();
	// Render final output
	finalPass();

	return true;
}

void App1::renderObjects(const XMMATRIX& view, const XMMATRIX& proj, std::unique_ptr<ShadowMap>* maps, bool renderDepth)
{
	//no need to pass it cause it's assigned in here anyways
	XMMATRIX world = renderer->getWorldMatrix();

	//floor
	renderTessellatedTerrain(mesh_floor.get(), world, view, proj, maps, textureMgr->getTexture(L"marble_diffuse"), textureMgr->getTexture(L"marble_height"), textureMgr->getTexture(L"marble_normal"), renderDepth);

	//Terrain Display
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(0.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(16.f, 2.f, 16.f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.f, 0.f, -15.f));
	renderTessellatedModel(model_pillar.get(), world, view, proj, XMFLOAT3(4.f, 4.f, 4.f), maps, textureMgr->getTexture(L"model_mei_pillar"),
		NULL, NULL, renderDepth);
	//world = XMMatrixScaling(.125f, .125f, .125f);
	//world = XMMatrixMultiply(world, XMMatrixTranslation(0.f, 2.75f, -15.f));
	world = renderer->getWorldMatrix();
	renderTessellatedTerrain(mesh_terrain.get(), world, view, proj, maps, textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"heightMap"), NULL, renderDepth);

	//Bench
	world = XMMatrixScaling(0.0675f, 0.0675f, 0.0675f);
	world = XMMatrixMultiply(world, XMMatrixTranslation(17.5f, 1.f, 13.f));
	renderTessellatedModel(model_bench.get(), world, view, proj, XMFLOAT3(0.0675f, 0.0675f, 0.0675f), maps, textureMgr->getTexture(L"model_bench_diffuse"),
		NULL, NULL, renderDepth);

	//Mei Display
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(0.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(4.f, 4.f, 4.f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(10.f, 0.f, 0.f));
	renderTessellatedModel(model_pillar.get(), world, view, proj, XMFLOAT3(4.f, 4.f, 4.f), maps, textureMgr->getTexture(L"model_mei_pillar"),
		NULL, NULL, renderDepth);
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(95.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(0.075f, 0.075f, 0.075f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(11.125f, 5.25f, 0.f));
	renderTessellatedModel(model_mei.get(), world, view, proj, XMFLOAT3(0.075f, 0.075f, 0.075f), maps, textureMgr->getTexture(L"model_mei_diffuse"),
		NULL, NULL, renderDepth);

	//Rock Display
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(0.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(4.f, 4.f, 4.f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.f, 0.f, 10.f));
	renderTessellatedModel(model_pillar.get(), world, view, proj, XMFLOAT3(4.f, 4.f, 4.f), maps, textureMgr->getTexture(L"model_mei_pillar"),
		NULL, NULL, renderDepth);
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 0.f, 1.f, 1.f), AI_DEG_TO_RAD(90.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(0.075f, 0.075f, 0.075f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.f, 6.25f, 10.25f));
	renderTessellatedModel(model_rock.get(), world, view, proj, XMFLOAT3(0.075f, 0.075f, 0.075f), maps, textureMgr->getTexture(L"model_rock_diffuse"),
		textureMgr->getTexture(L"model_rock_normal"), textureMgr->getTexture(L"model_rock_height"), renderDepth);

	//Totoro Display
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(0.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(4.f, 4.f, 4.f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(-10.f, 0.f, 0.f));
	renderTessellatedModel(model_pillar.get(), world, view, proj, XMFLOAT3(4.f, 4.f, 4.f), maps, textureMgr->getTexture(L"model_mei_pillar"),
		NULL, NULL, renderDepth);
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(-90.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(4.f, 4.f, 4.f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(-10.f, 5.25f, 0.f));
	renderTessellatedModel(model_totoro.get(), world, view, proj, XMFLOAT3(4.f, 4.f, 4.f), maps, textureMgr->getTexture(L"model_totoro_diffuse"),
		NULL, NULL, renderDepth);
}

void App1::renderTessellatedModel(TModel* model, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj, const XMFLOAT3& scale,
	std::unique_ptr<ShadowMap>* maps, ID3D11ShaderResourceView* texture_diffuse, ID3D11ShaderResourceView* texture_normal,
	ID3D11ShaderResourceView* texture_height, bool renderDepth)
{
	model->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	if (renderDepth)
	{
		depth_tess_model_shader->setShaderParameters(
			renderer->getDeviceContext(), world, view, proj, model, camera);
		depth_tess_model_shader->render(renderer->getDeviceContext(), model->getIndexCount());
	}
	else
	{
		model_tess_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj, model, maps, light.data(), camera, gui_render_normals);
		model_tess_shader->render(renderer->getDeviceContext(), model->getIndexCount());
	}
}

void App1::renderTessellatedTerrain(TerrainMesh* terrain, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj,
	std::unique_ptr<ShadowMap>* maps, ID3D11ShaderResourceView* texture_diffuse, ID3D11ShaderResourceView* heightmap, ID3D11ShaderResourceView* normalmap, bool renderDepth)
{
	terrain->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depth_tess_terrain_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj, terrain, camera);
		depth_tess_terrain_shader->render(renderer->getDeviceContext(), terrain->getIndexCount());
	}
	else
	{
		terrain_tess_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj, terrain, maps, light.data(), camera, gui_render_normals);
		terrain_tess_shader->render(renderer->getDeviceContext(), terrain->getIndexCount());

		/*grass_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"heightMap"), gui_min_max_LOD, gui_min_max_distance, gui_terrain_height_amplitude, camera);
		grass_shader->render(renderer->getDeviceContext(), mesh_terrain->getIndexCount());*/
	}
}

void App1::depthPass()
{
	//Do depths passes for every lights
	for (int i = 0, mapID = 0; i < N_LIGHTS; ++i, mapID += 6)
	{
		int type = light[i]->getType();

		//Do not create a shadowMap if the light is off
		if (!type) continue;

		//point lights need a cube map
		if (type == 2)
		{
			//looping 6 times, for each face of the shadow cube map (point lights need cube maps)
			for (int j = 0; j < 6; ++j)
			{
				// Set the render target to be the render to texture.
				shadowmap[mapID + j]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

				// get the world, view, and projection matrices from the camera and d3d objects.
				XMMATRIX lightViewMatrix = light[i]->getPointViewMatrix(j);
				XMMATRIX lightProjectionMatrix = light[i]->getProjectionMatrix();

				// Render other objects used in multiple passes
				renderObjects(lightViewMatrix, lightProjectionMatrix, nullptr, true);
			}
		}
		else
		{
			// Set the render target to be the render to texture.
			shadowmap[mapID]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

			// get the world, view, and projection matrices from the camera and d3d objects.
			XMMATRIX lightViewMatrix = light[i]->getViewMatrix();
			XMMATRIX lightProjectionMatrix = type == 3 ? light[i]->getProjectionMatrix() : light[i]->getOrthoMatrix();

			// Render other objects used in multiple passes
			renderObjects(lightViewMatrix, lightProjectionMatrix, nullptr, true);
		}
	}
	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::firstPass()
{
	// In the first pass, render the entire scene to a render target, so we can use it for the bloom post-processing effect
	bloom_scene_render_target->setRenderTarget(renderer->getDeviceContext());
	bloom_scene_render_target->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	//Render the light debug sphere
	if (gui_render_light_sphere)
	{
		mesh_light_debug_sphere->sendData(renderer->getDeviceContext());
		for (int i = 0; i < N_LIGHTS; ++i)
		{
			//Get the type of the light
			int type = light[i]->getType();

			//If light is off, dont draw sphere
			if (!type) continue;

			XMFLOAT3 light_pos = light[i]->getPosition();
			type == 1 ? worldMatrix = XMMatrixScaling(10.f, 10.f, 10.f) : worldMatrix = XMMatrixScaling(.5f, .5f, .5f);
			worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(light_pos.x, light_pos.y, light_pos.z));
			light_debug_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, light[i].get());
			light_debug_shader->render(renderer->getDeviceContext(), mesh_light_debug_sphere->getIndexCount());
		}
	}

	// Render other objects used in multiple passes
	renderObjects(viewMatrix, projectionMatrix, shadowmap.data(), false);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::bloomPass()
{
	//Get the pixels that are above the treshhold value
	bloom_threshold_compute->setShaderParameters(renderer->getDeviceContext(), bloom_scene_render_target->getShaderResourceView(), gui_bloom_threshold);
	bloom_threshold_compute->compute(renderer->getDeviceContext(), ceil((float)sWidth / 16.f), ceil((float)sHeight / 16.f), 1);
	bloom_threshold_compute->unbind(renderer->getDeviceContext());

	ID3D11ShaderResourceView* buffer_to_blur = bloom_threshold_compute->getShaderResourceView();
	for (int i = 0; i < gui_bloom_blur_iterations; ++i)
	{
		// horiontal pass on bloom target
		horizontal_blur_compute->setShaderParameters(renderer->getDeviceContext(), buffer_to_blur);
		horizontal_blur_compute->compute(renderer->getDeviceContext(), ceil((float)sWidth / 256.f), sHeight, 1);
		//ceil((float)sWidth / 256.f) why? Because in the compute shader file, N = 256 threads will be created for each thread group.
		//Therefore the width is divided to make sure each thread will have some work to do
		horizontal_blur_compute->unbind(renderer->getDeviceContext());

		// Vertical blur using the horizontal blur result
		vertical_blur_compute->setShaderParameters(renderer->getDeviceContext(), horizontal_blur_compute->getShaderResourceView());
		vertical_blur_compute->compute(renderer->getDeviceContext(), sWidth, ceil((float)sHeight / 256.f), 1);
		vertical_blur_compute->unbind(renderer->getDeviceContext());

		buffer_to_blur = vertical_blur_compute->getShaderResourceView();
	}

	bloom_combine_compute->setShaderParameters(renderer->getDeviceContext(), bloom_scene_render_target->getShaderResourceView(), buffer_to_blur);
	bloom_combine_compute->compute(renderer->getDeviceContext(), ceil((float)sWidth / 16.f), ceil((float)sHeight / 16.f), 1);
	bloom_combine_compute->unbind(renderer->getDeviceContext());

	//The below compute shader was a bad attempt at combining all the above compute shaders into a single one
	//Had artefacts, performed worse -> discarded
	/*bloom_compute->setShaderParameters(renderer->getDeviceContext(), bloom_scene_render_target->getShaderResourceView(), gui_bloom_threshold, gui_bloom_blur_iterations);
	bloom_compute->compute(renderer->getDeviceContext(), ceil((float)sWidth / 16.f), ceil((float)sHeight / 16.f), 1);
	bloom_compute->unbind(renderer->getDeviceContext());*/

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	//// Render 2D meshes
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();

	//Make sure to not render these meshes in wireframe even if it is toggled on, otherwise we won't see anything (destroys the purpose)
	renderer->setWireframeMode(false);

	//Render the main scene after post processing
	renderer->setZBuffer(false);
	orthomesh_display->sendData(renderer->getDeviceContext());
	texture_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, gui_bloom_enable ? bloom_combine_compute->getShaderResourceView() : bloom_scene_render_target->getShaderResourceView());
	texture_shader->render(renderer->getDeviceContext(), orthomesh_display->getIndexCount());
	renderer->setZBuffer(true);

	//Render the shadow map outputs
	renderer->setZBuffer(false);
	orthomesh_debug_shadow_maps->sendData(renderer->getDeviceContext());
	texture_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowmap[gui_shadow_map_display_index]->getDepthMapSRV());
	texture_shader->render(renderer->getDeviceContext(), orthomesh_debug_shadow_maps->getIndexCount());
	renderer->setZBuffer(true);


	//// Render GUI
	gui();
	renderer->endScene();
}

void App1::gui()
{
	auto generateMatrix = [&](int index)->void
	{
		int type = light[index]->getType();

		type == 2 ?
			light[index]->generatePointLightViewMatrices() :
			light[index]->generateViewMatrix();

		type == 1 ?
			light[index]->generateOrthoMatrix(gui_light_scene_dimensions[index].x, gui_light_scene_dimensions[index].y, gui_light_frustum[index].x, gui_light_frustum[index].y) :
			light[index]->generateProjectionMatrix(gui_light_frustum[index].x, gui_light_frustum[index].y);
	};

	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	ImGui::ShowDemoWindow();

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Render Normals", &gui_render_normals);

	ImGui::Separator();

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * .5f);
	ImGui::SliderInt("Display Shadow Map Number", &gui_shadow_map_display_index, 0, 6 * N_LIGHTS);
	ImGui::PopItemWidth();


	if (ImGui::CollapsingHeader("Terrains Settings"))
	{
		if (ImGui::TreeNode("Floor"))
		{
			ImGui::PushID(0);

			ImGui::Checkbox("Use Normal Map", mesh_floor->getPtrNormalMap());
			ImGui::SliderFloat2("minMaxLOD", &mesh_floor->getPtrMinMaxLOD()->x, 0.f, 100.f);
			ImGui::SliderFloat2("minMaxDistance", &mesh_floor->getPtrMinMaxDistance()->x, 0.f, 200.f);
			ImGui::SliderFloat2("Texture Scale", &mesh_floor->getPtrTextureScale()->x, .1f, 20.f);
			ImGui::SliderFloat("Height Amplitude", mesh_floor->getPtrHeightAmplitude(), 0.f, 20.f);

			ImGui::PopID();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Terrain On Display"))
		{
			ImGui::PushID(1);

			ImGui::Text("This terrain does not have any normal map!");
			ImGui::SliderFloat2("minMaxLOD", &mesh_terrain->getPtrMinMaxLOD()->x, 0.f, 100.f);
			ImGui::SliderFloat2("minMaxDistance", &mesh_terrain->getPtrMinMaxDistance()->x, 0.f, 200.f);
			ImGui::SliderFloat2("Texture Scale", &mesh_terrain->getPtrTextureScale()->x, .1f, 20.f);
			ImGui::SliderFloat("Height Amplitude", mesh_terrain->getPtrHeightAmplitude(), 0.f, 20.f);
			
			ImGui::PopID();
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Model Mesh Settings:"))
	{
		if (ImGui::TreeNode("Pillar"))
		{
			ImGui::PushID(0);

			ImGui::Checkbox("Use Normal Map", model_pillar->getPtrNormalMap());
			ImGui::SliderFloat("Tessellation Factor Inside", &model_pillar->getPtrTessellationFactors()->x, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Factor Outside", &model_pillar->getPtrTessellationFactors()->y, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Height Amplitude", model_pillar->getPtrHeightAmplitude(), 0.f, 20.f);

			ImGui::PopID();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Bench"))
		{
			ImGui::PushID(2);

			ImGui::Checkbox("Use Normal Map", model_bench->getPtrNormalMap());
			ImGui::SliderFloat("Tessellation Factor Inside", &model_bench->getPtrTessellationFactors()->x, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Factor Outside", &model_bench->getPtrTessellationFactors()->y, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Height Amplitude", model_bench->getPtrHeightAmplitude(), 0.f, 20.f);

			ImGui::PopID();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Rock"))
		{
			ImGui::PushID(3);

			ImGui::Checkbox("Use Normal Map", model_rock->getPtrNormalMap());
			ImGui::SliderFloat("Tessellation Factor Inside", &model_rock->getPtrTessellationFactors()->x, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Factor Outside", &model_rock->getPtrTessellationFactors()->y, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Height Amplitude", model_rock->getPtrHeightAmplitude(), 0.f, 20.f);

			ImGui::PopID();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Mei"))
		{
			ImGui::PushID(4);

			ImGui::Text("This model does not have a normal map!");
			ImGui::SliderFloat("Tessellation Factor Inside", &model_mei->getPtrTessellationFactors()->x, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Factor Outside", &model_mei->getPtrTessellationFactors()->y, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Height Amplitude", model_mei->getPtrHeightAmplitude(), 0.f, 20.f);

			ImGui::PopID();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Totoro"))
		{
			ImGui::PushID(5);

			ImGui::Text("This model does not have a normal map!");
			ImGui::SliderFloat("Tessellation Factor Inside", &model_totoro->getPtrTessellationFactors()->x, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Factor Outside", &model_totoro->getPtrTessellationFactors()->y, 1.f, 64.f);
			ImGui::SliderFloat("Tessellation Height Amplitude", model_totoro->getPtrHeightAmplitude(), 0.f, 20.f);

			ImGui::PopID();
			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Bloom Settings:"))
	{
		ImGui::Checkbox("Enable Bloom Effect", &gui_bloom_enable);
		ImGui::DragFloat("Threshold", &gui_bloom_threshold, 0.001f, 0.f, 10.f);
		ImGui::SliderInt("Blur Passes", &gui_bloom_blur_iterations, 1, 20);
	}

	static int open_light_editor = 0;
	open_light_editor += ImGui::Selectable("Edit Lights", true);
	if (open_light_editor % 2)
	{
		ImGui::Begin("Light Editor", (bool*)&open_light_editor);
		ImGui::Checkbox("Render Debug Spheres", &gui_render_light_sphere);

		for (int i = 0; i < N_LIGHTS; ++i)
		{
			std::string header = "light " + std::to_string(i);
			if (ImGui::TreeNode(header.c_str()))
			{
				ImGui::PushID(i);
				ImGui::Separator();

				if (ImGui::RadioButton("OFF", &gui_light_type[i], 0))
					light[i]->setType(gui_light_type[i]);
				if (ImGui::RadioButton("Directional", &gui_light_type[i], 1))
				{
					light[i]->setType(gui_light_type[i]);
					//light[i]->setDirection(gui_light_direction[i].x, gui_light_direction[i].y, gui_light_direction[i].z);	//reset direction in case we're moving from a point light
					generateMatrix(i);
				}
				if (ImGui::RadioButton("Point", &gui_light_type[i], 2))
				{
					light[i]->setType(gui_light_type[i]);
					generateMatrix(i);
				}
				if (ImGui::RadioButton("Spot", &gui_light_type[i], 3))
				{
					light[i]->setType(gui_light_type[i]);
					//light[i]->setDirection(gui_light_direction[i].x, gui_light_direction[i].y, gui_light_direction[i].z);	//reset direction in case we're moving from a point light
					generateMatrix(i);
				}
				if (ImGui::SliderFloat2("Scene Dimensions", &gui_light_scene_dimensions[i].x, 1.f, 1000.f))
					generateMatrix(i);
				if (ImGui::SliderFloat3("Light Direction", &gui_light_direction[i].x, -1.f, 1.f))
				{
					light[i]->setDirection(gui_light_direction[i].x, gui_light_direction[i].y, gui_light_direction[i].z);
					generateMatrix(i);
				}
				if (ImGui::SliderFloat3("Light Position", &gui_light_position[i].x, -100.f, 100.f))
				{
					light[i]->setPosition(gui_light_position[i].x, gui_light_position[i].y, gui_light_position[i].z);
					generateMatrix(i);
				}
				if (ImGui::SliderFloat("Near Plane", &gui_light_frustum[i].x, 0.01f, 100.f))
					generateMatrix(i);
				if (ImGui::SliderFloat("Far Plane", &gui_light_frustum[i].y, 0.f, 1000.f))
					generateMatrix(i);
				if (ImGui::SliderFloat("Light Intensity", &gui_light_intensity[i], 0.f, 1.f))
					light[i]->setIntensity(gui_light_intensity[i]);
				if (ImGui::ColorEdit4("Ambient Colour", &gui_light_ambient_colour[i].x, ImGuiColorEditFlags_Float))
					light[i]->setAmbientColour(gui_light_ambient_colour[i].x, gui_light_ambient_colour[i].y, gui_light_ambient_colour[i].z, gui_light_ambient_colour[i].w);
				if (ImGui::ColorEdit4("Diffuse Colour", &gui_light_diffuse_colour[i].x, ImGuiColorEditFlags_Float))
				{
					light[i]->setDiffuseColour(gui_light_diffuse_colour[i].x, gui_light_diffuse_colour[i].y, gui_light_diffuse_colour[i].z, gui_light_diffuse_colour[i].w);
					gui_light_specular_sameAsDiffuse[i] ?
						light[i]->setSpecularColour(gui_light_diffuse_colour[i].x, gui_light_diffuse_colour[i].y, gui_light_diffuse_colour[i].z, gui_light_diffuse_colour[i].w) :
						light[i]->setSpecularColour(gui_light_specular_colour[i].x, gui_light_specular_colour[i].y, gui_light_specular_colour[i].z, gui_light_specular_colour[i].w);
				}
				if (ImGui::SliderFloat3("Attenuation Factors", &gui_light_attenuation_factors[i].x, 0.f, 1.f))
					light[i]->setAttenuationFactors(gui_light_attenuation_factors[i]);
				if (ImGui::SliderFloat("Specular Power", &gui_light_specular_power[i], 0.f, 1000.f))
					light[i]->setSpecularPower(gui_light_specular_power[i]);
				if(ImGui::Checkbox("Use Same Specular Colour As Diffuse Colour", &gui_light_specular_sameAsDiffuse[i]))
					gui_light_specular_sameAsDiffuse[i] ?
					light[i]->setSpecularColour(gui_light_diffuse_colour[i].x, gui_light_diffuse_colour[i].y, gui_light_diffuse_colour[i].z, gui_light_diffuse_colour[i].w) :
					light[i]->setSpecularColour(gui_light_specular_colour[i].x, gui_light_specular_colour[i].y, gui_light_specular_colour[i].z, gui_light_specular_colour[i].w);
				if (ImGui::ColorEdit4("Specular Colour", &gui_light_specular_colour[i].x, ImGuiColorEditFlags_Float))
					gui_light_specular_sameAsDiffuse[i] ?
					light[i]->setSpecularColour(gui_light_diffuse_colour[i].x, gui_light_diffuse_colour[i].y, gui_light_diffuse_colour[i].z, gui_light_diffuse_colour[i].w) :
					light[i]->setSpecularColour(gui_light_specular_colour[i].x, gui_light_specular_colour[i].y, gui_light_specular_colour[i].z, gui_light_specular_colour[i].w);
				//Spotlight specific settings, hidden when other types
				if (gui_light_type[i] == 3)
				{
					ImGui::Separator();
					ImGui::Text("Spot Light Specific Settings");
					if (ImGui::SliderFloat("Falloff", &gui_light_spot_falloff[i], 0.f, 256.f))
						light[i]->setSpotFalloff(gui_light_spot_falloff[i]);
					if (ImGui::SliderFloat("Angle", &gui_light_spot_angle[i], 0.f, 90.f))
						light[i]->setSpotAngle(gui_light_spot_angle[i]);
				}
				ImGui::Separator();
				ImGui::Text("Shadow Settings");
				if (ImGui::SliderFloat("Shadow Bias", &gui_light_shadow_bias[i], 0.f, 0.01f))
					light[i]->setShadowBias(gui_light_shadow_bias[i]);

				ImGui::Separator();
				ImGui::PopID();
				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

