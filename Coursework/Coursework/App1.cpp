// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1() : gui_animate_objects(false), objects_roation_angle(0.f), gui_shadow_map_display_index(0)
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
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = std::make_unique<PlaneMesh>(renderer->getDevice(), renderer->getDeviceContext());
	model = std::make_unique<AModel>(renderer->getDevice(), "res/teapot.obj");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wood", L"res/wood.png");

	// initial shaders
	textureShader = std::make_unique<TextureShader>(renderer->getDevice(), hwnd);
	depthShader = std::make_unique<DepthShader>(renderer->getDevice(), hwnd);
	shadowShader = std::make_unique<ShadowShader>(renderer->getDevice(), hwnd);
	lightDebugShader = std::make_unique<LightDebugShader>(renderer->getDevice(), hwnd);

	// Variables for defining shadow map
	int shadowmapWidth = 1024;	//MAX: 16'384
	int shadowmapHeight = 1024;

	for(int i = 0; i < N_LIGHTS * 6; ++i)
		//Create as much shadowMaps as there are lights * 6 (for point lights)
		shadowMap[i] = std::make_unique<ShadowMap>(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

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
	orthoMesh = std::make_unique<OrthoMesh>(renderer->getDevice(), renderer->getDeviceContext(), screenWidth / 4, screenHeight / 4, screenWidth / 3, screenHeight / 3);

	//Init additional objects
	cube = std::make_unique<CubeMesh>(renderer->getDevice(), renderer->getDeviceContext());
	sphere = std::make_unique<SphereMesh>(renderer->getDevice(), renderer->getDeviceContext());
	light_debug_sphere = std::make_unique<SphereMesh>(renderer->getDevice(), renderer->getDeviceContext());
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
	objects_roation_angle += (float)gui_animate_objects * 10.f * timer->getTime();
	objects_roation_angle -= (objects_roation_angle > 360.f) * 360.f;
	
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
	XMMATRIX world;

	//floor
	world = XMMatrixTranslation(-50.f, 0.f, -10.f);
	mesh->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depthShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());
	}
	else
	{
		shadowShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"brick"), maps, light.data(), camera);
		shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());
	}

	//teapot
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(objects_roation_angle));
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.f, 7.f, 5.f));
	world = XMMatrixMultiply(world, XMMatrixScaling(0.5f, 0.5f, 0.5f));
	world = XMMatrixMultiply(world, XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(objects_roation_angle)));
	model->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depthShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depthShader->render(renderer->getDeviceContext(), model->getIndexCount());
	}
	else
	{
		shadowShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"wood"), maps, light.data(), camera);
		shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());
	}

	//cube
	world = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(objects_roation_angle));
	world = XMMatrixMultiply(world, XMMatrixTranslation(15.f, 1.f, 5.f));
	world = XMMatrixMultiply(world, XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 1.f), AI_DEG_TO_RAD(objects_roation_angle)));
	cube->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depthShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());
	}
	else
	{
		shadowShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"wood"), maps, light.data(), camera);
		shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());
	}

	//sphere
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.f, 0.f, 5.f));
	sphere->sendData(renderer->getDeviceContext());
	if (renderDepth)
	{
		depthShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj);
		depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());
	}
	else
	{
		shadowShader->setShaderParameters(renderer->getDeviceContext(), world, view, proj,
			textureMgr->getTexture(L"wood"), maps, light.data(), camera);
		shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());
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
				shadowMap[mapID + j]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

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
			shadowMap[mapID]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

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
	light_debug_sphere->sendData(renderer->getDeviceContext());
	for (int i = 0; i < N_LIGHTS; ++i)
	{
		//If light is off, dont draw sphere
		if (!light[i]->getType()) continue;

		XMFLOAT3 light_pos = light[i]->getPosition();
		worldMatrix = XMMatrixScaling(.5f, .5f, .5f);
		worldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixTranslation(light_pos.x, light_pos.y, light_pos.z));
		lightDebugShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, light[i].get());
		lightDebugShader->render(renderer->getDeviceContext(), light_debug_sphere->getIndexCount());
	}

	// Render other objects used in multiple passes
	renderObjects(viewMatrix, projectionMatrix, shadowMap.data(), false);

	//Render 2D meshes
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getOrthoViewMatrix();
	projectionMatrix = renderer->getOrthoMatrix();
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, shadowMap[gui_shadow_map_display_index]->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
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
	ImGui::Checkbox("Animate Objects", &gui_animate_objects);

	ImGui::Separator();

	ImGui::PushItemWidth(ImGui::GetWindowWidth() * .5f);
	ImGui::SliderInt("Display Shadow Map Number", &gui_shadow_map_display_index, 0, 6 * N_LIGHTS);
	ImGui::PopItemWidth();

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

