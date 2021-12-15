// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1() : gui_shadow_map_display_index(0), gui_min_max_LOD(1.f, 15.f), gui_min_max_distance(50.f, 75.f),
gui_render_normals(false), gui_terrain_texture_sacale(XMFLOAT2(20.f, 20.f)), gui_terrain_height_amplitude(2.875f)
{
	for (int i = 0; i < N_LIGHTS; ++i)
	{
		gui_light_drop_menu_activate[i] = false;
		gui_light_scene_dimensions[i] = XMFLOAT2(100.f, 100.f);
		gui_light_direction[i] = XMFLOAT3(0.f, -.7f, .7f);
		gui_light_position[i] = XMFLOAT3(0.f, 0.f, -10.f);
		gui_light_frustum[i] = XMFLOAT2(.1f, 100.f);
		gui_light_diffuse_colour[i] = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
		gui_light_ambient_colour[i] = XMFLOAT4(.3f, .3f, .3f, 1.f);
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
	gui_light_type[0] = 1;	//Only the first light is on by default
	gui_light_direction[0] = XMFLOAT3(.25f, -.125f, .7f);
	gui_light_position[0] = XMFLOAT3(-50.f, 30.f, -125.f);
	gui_light_frustum[0] = XMFLOAT2(45.f, 225.f);
	gui_light_diffuse_colour[0] = XMFLOAT4(.995f, .586f, .0f, 1.f);
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"heightMap", L"res/heightmap7.dds");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wood", L"res/wood.png");
	textureMgr->loadTexture(L"grass", L"res/grass_2.jpg");
	textureMgr->loadTexture(L"model_mei_diffuse", L"res/models/mei/Mei_TEX.png");
	textureMgr->loadTexture(L"model_totoro_diffuse", L"res/models/totoro/Totoro_Map.png");
	textureMgr->loadTexture(L"model_rock_diffuse", L"res/models/rock_model_1/textures/Rock_1_Diffuse.jpg");
	textureMgr->loadTexture(L"model_rock_height", L"res/models/rock_model_1/textures/Rock_1_Glossiness.jpg");
	textureMgr->loadTexture(L"model_rock_normal", L"res/models/rock_model_1/textures/Rock_1_Normal.jpg");
	textureMgr->loadTexture(L"model_bench_diffuse", L"res/models/bench/benchs_diffuse.png");
	textureMgr->loadTexture(L"model_bench_normal", L"res/models/bench/benchs_normal.png");
	textureMgr->loadTexture(L"model_lamp_diffuse", L"res/models/lamp2/diffuse.png");

	// Init shaders
	texture_shader = std::make_unique<TextureShader>(renderer->getDevice(), hwnd);
	depth_shader = std::make_unique<DepthShader>(renderer->getDevice(), hwnd);
	depth_tess_shader = std::make_unique<DepthTessShader>(renderer->getDevice(), hwnd);
	light_shader = std::make_unique<ShadowShader>(renderer->getDevice(), hwnd);
	light_debug_shader = std::make_unique<LightDebugShader>(renderer->getDevice(), hwnd);
	terrain_tess_shader = std::make_unique<PlaneTessellationShader>(renderer->getDevice(), hwnd);
	model_tess_shader = std::make_unique<ModelTessellationShader>(renderer->getDevice(), hwnd);
	debug_normals_shader = std::make_unique<DebugNormalsShader>(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 4096;	//MAX: 16'384
	int shadowmapHeight = 4096;

	for(int i = 0; i < N_LIGHTS * 6; ++i)
		//Create as much shadowMaps as there are lights * 6 (for point lights)
		shadowmap[i] = std::make_unique<ShadowMap>(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	for (int i = 0; i < N_LIGHTS; ++i)
	{
		//Init lights
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

	//Init orthomesh
	mesh_orthomesh = std::make_unique<OrthoMesh>(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 3, screenHeight / 3);

	//Init additional objects
	mesh_light_debug_sphere = std::make_unique<SphereMesh>(renderer->getDevice(), renderer->getDeviceContext());
	mesh_terrain = std::make_unique<TerrainMesh>(renderer->getDevice(), renderer->getDeviceContext());
	model_mei = std::make_unique<AModel>(renderer->getDevice(), "res/models/mei/Mei_Run.fbx");
	model_totoro = std::make_unique<AModel>(renderer->getDevice(), "res/models/totoro/totoro.fbx");
	model_rock = std::make_unique<AModel>(renderer->getDevice(), "res/models/rock_model_1/rock.obj");
	model_bench = std::make_unique<AModel>(renderer->getDevice(), "res/models/bench/bench.obj");
	model_lamp = std::make_unique<AModel>(renderer->getDevice(), "res/models/lamp2/vsl.obj");
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
	// Render scene
	finalPass();

	return true;
}

void App1::renderObjects(const XMMATRIX& view, const XMMATRIX& proj, std::unique_ptr<ShadowMap>* maps, bool renderDepth)
{
	//no need to pass it cause it's assigned in here anyways
	XMMATRIX world = renderer->getWorldMatrix();

	//floor
	world = XMMatrixTranslation(0.f, -20.f, 0.f);
	mesh_terrain->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depth_tess_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"heightMap"), gui_min_max_LOD, gui_min_max_distance, gui_terrain_height_amplitude, camera);
		depth_tess_shader->render(renderer->getDeviceContext(), mesh_terrain->getIndexCount());
	}
	else
	{
		terrain_tess_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"heightMap"), gui_min_max_LOD, gui_min_max_distance, gui_terrain_texture_sacale,
			gui_terrain_height_amplitude, maps, light.data(), camera, gui_render_normals);
		terrain_tess_shader->render(renderer->getDeviceContext(), mesh_terrain->getIndexCount());
	}

	//Rocks
	world = XMMatrixRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 1.f), AI_DEG_TO_RAD(90.f));
	world = XMMatrixMultiply(world, XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(90.f)));
	world = XMMatrixMultiply(world, XMMatrixScaling(0.5f, 0.25f, 0.5f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(2.f, -20.f, -6.f));
	model_rock->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depth_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depth_shader->render(renderer->getDeviceContext(), model_rock->getIndexCount());
	}
	else
	{
		light_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"model_rock_diffuse"),  maps, light.data(), camera);
		light_shader->render(renderer->getDeviceContext(), model_rock->getIndexCount());
		if (gui_render_normals)
		{
			debug_normals_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
			debug_normals_shader->render(renderer->getDeviceContext(), model_rock->getIndexCount());
		}
	}
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 0.f, 1.f, 1.f), AI_DEG_TO_RAD(90.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(0.375f, 0.125f, 0.25f));
	world = XMMatrixMultiply(world, XMMatrixTranslation(-12.f, -19.f, 12.f));
	model_rock->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depth_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depth_shader->render(renderer->getDeviceContext(), model_rock->getIndexCount());
	}
	else
	{
		light_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"model_rock_diffuse"), maps, light.data(), camera);
		light_shader->render(renderer->getDeviceContext(), model_rock->getIndexCount());
		if (gui_render_normals)
		{
			debug_normals_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
			debug_normals_shader->render(renderer->getDeviceContext(), model_rock->getIndexCount());
		}
	}

	//Bench
	world = XMMatrixScaling(0.0675f, 0.0675f, 0.0675f);
	world = XMMatrixMultiply(world, XMMatrixTranslation(17.5f, -19.125f, 13.f));
	model_bench->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depth_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depth_shader->render(renderer->getDeviceContext(), model_bench->getIndexCount());
	}
	else
	{
		light_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"model_bench_diffuse"), maps, light.data(), camera);
		light_shader->render(renderer->getDeviceContext(), model_bench->getIndexCount());
		if (gui_render_normals)
		{
			debug_normals_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
			debug_normals_shader->render(renderer->getDeviceContext(), model_bench->getIndexCount());
		}
	}

	//Lamp
	world = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	world = XMMatrixMultiply(world, XMMatrixTranslation(10.f, 0.f, 13.f));
	model_lamp->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depth_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depth_shader->render(renderer->getDeviceContext(), model_lamp->getIndexCount());
	}
	else
	{
		light_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"model_lamp_diffuse"), maps, light.data(), camera);
		light_shader->render(renderer->getDeviceContext(), model_lamp->getIndexCount());
		if (gui_render_normals)
		{
			debug_normals_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
			debug_normals_shader->render(renderer->getDeviceContext(), model_lamp->getIndexCount());
		}
	}

	//Mei
	//world = XMMatrixScaling(0.0675f, 0.0675f, 0.0675f);
	////world = XMMatrixMultiply(world, XMMatrixTranslation(20.f, -6.5f, 13.f));
	//model_mei->sendData(renderer->getDeviceContext());
	//if (renderDepth)
	//{
	//	depth_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
	//	depth_shader->render(renderer->getDeviceContext(), model_mei->getIndexCount());
	//}
	//else
	//{
	//	light_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
	//		textureMgr->getTexture(L"model_mei_diffuse"), maps, light.data(), camera);
	//	light_shader->render(renderer->getDeviceContext(), model_mei->getIndexCount());
	//	if (gui_render_normals)
	//	{
	//		debug_normals_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
	//		debug_normals_shader->render(renderer->getDeviceContext(), model_mei->getIndexCount());
	//	}
	//}

	//Totoro
	world = XMMatrixScaling(15.f, 15.f, 15.f);
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.f, -20.f, 13.f));
	model_totoro->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depth_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depth_shader->render(renderer->getDeviceContext(), model_totoro->getIndexCount());
	}
	else
	{
		light_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"model_totoro_diffuse"), maps, light.data(), camera);
		light_shader->render(renderer->getDeviceContext(), model_totoro->getIndexCount());
		if (gui_render_normals)
		{
			debug_normals_shader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
			debug_normals_shader->render(renderer->getDeviceContext(), model_totoro->getIndexCount());
		}
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

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	//Render the light debug sphere
	mesh_light_debug_sphere->sendData(renderer->getDeviceContext());
	for (int i = 0; i < N_LIGHTS; ++i)
	{
		//If light is off, dont draw sphere
		if (!light[i]->getType()) continue;

		XMFLOAT3 light_pos = light[i]->getPosition();
		worldMatrix = XMMatrixScaling(.5f, .5f, .5f);
		worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(light_pos.x, light_pos.y, light_pos.z));
		light_debug_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, light[i].get());
		light_debug_shader->render(renderer->getDeviceContext(), mesh_light_debug_sphere->getIndexCount());
	}

	// Render other objects used in multiple passes
	renderObjects(viewMatrix, projectionMatrix, shadowmap.data(), false);

	//Render 2D meshes
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getOrthoViewMatrix();
	projectionMatrix = renderer->getOrthoMatrix();
	renderer->setZBuffer(false);
	mesh_orthomesh->sendData(renderer->getDeviceContext());
	texture_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, shadowmap[gui_shadow_map_display_index]->getDepthMapSRV());
	texture_shader->render(renderer->getDeviceContext(), mesh_orthomesh->getIndexCount());
	renderer->setZBuffer(true);

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

	ImGui::Separator();

	ImGui::Text("Terrain Mesh Settings:");
	ImGui::SliderFloat2("minMaxLOD", &gui_min_max_LOD.x, 0.f, 100.f);
	ImGui::SliderFloat2("minMaxDistance", &gui_min_max_distance.x, 0.f, 200.f);
	ImGui::SliderFloat2("Texture Scale", &gui_terrain_texture_sacale.x, .1f, 20.f);
	ImGui::SliderFloat("Height Amplitude", &gui_terrain_height_amplitude, 0.f, 20.f);

	ImGui::Separator();

	static int open_light_editor = 0;
	open_light_editor += ImGui::Selectable("Edit Lights", true);
	if (open_light_editor % 2)
	{
		ImGui::Begin("Light Editor", (bool*)&open_light_editor);

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

