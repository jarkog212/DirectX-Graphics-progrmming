#include "App1.h"
#include "ShaderUtils.h"
#include "BetterPointMesh.h"

App1::App1()
{
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	//! Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	//! store screen resolution for PosstProcessing
	resolution_.x = (float)screenWidth;
	resolution_.y = (float)screenHeight;
	
	//! initialise material lbrary
	materialLib_ = new MaterialLibrary(textureMgr, renderer);

	//! Build RenderTexture, this will be our alternative render target.
	renderTexture_ = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	//! Build depth texture - uses ShadowMap technology
	depthTexture_ = new ShadowMap(renderer->getDevice(), resolution_.x, resolution_.y);

	//! Initalise shaders.
	defaultShader_ = new DefaultShader(renderer->getDevice(), hwnd);
	landscapeShader_ = new LandscapeShader(renderer->getDevice(), hwnd);
	foliageShader_ = new FoliageShader(renderer->getDevice(), hwnd);
	waterShader_ = new WaterShader(renderer->getDevice(), hwnd);
	gpuOrderShader_ = new GPUOrderShader(renderer->getDevice(), hwnd);
	windShader_ = new WindShader(renderer->getDevice(), hwnd);
	PPBlurShader_ = new PPBlurShader(renderer->getDevice(), hwnd);
	PPDofShader_ = new PPDofShader(renderer->getDevice(), hwnd);
	simpleShader_ = new SimpleShader(renderer->getDevice(), hwnd);
	
	//! render ortho mesh initialisation
	orthoMesh_ = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight, 0, 0);

	//! landcape intitialisation
	initLandscape();

	//! River / water
	initWater();

	//! scene objects
	sceneObjects_.push_back(new Object(new SphereMesh(renderer->getDevice(), renderer->getDeviceContext()), defaultShader_, simpleShader_, NULL , NULL, materialLib_->getMaterial("Test")));
	sceneObjects_.back()->setObjectTransform({ 2,0,0 });
	
	sceneObjects_.push_back(new Object(new CubeMesh(renderer->getDevice(), renderer->getDeviceContext()), defaultShader_, simpleShader_, NULL, NULL, materialLib_->getMaterial("Test")));
	sceneObjects_.back()->setObjectTransform({ -2,0,0 });

	//! wind models
	//! responsibility for the heap struct is given to the object
	initWind();

	sceneObjects_.push_back(new Object(materialLib_->getMesh("Tree"), windShader_, NULL, textureMgr->getTexture(L"tree3D"), NULL, materialLib_->getMaterial("Base"), D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST));
	sceneObjects_.back()->setObjectTransform({ 59.72,14.7,67.63 }, { 0,15,0 }, {20,20,20});
	sceneObjects_.back()->setAdditionalShaderData(windParams);

	sceneObjects_.push_back(new Object(materialLib_->getMesh("Tree"), windShader_, NULL, textureMgr->getTexture(L"tree3D"), NULL, materialLib_->getMaterial("Base"), D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST));
	sceneObjects_.back()->setObjectTransform({ 0,0,0 }, { 0,0,0 }, { 20,20,20 });
	sceneObjects_.back()->setAdditionalShaderData(windParams,false);

	sceneObjects_.push_back(new Object(materialLib_->getMesh("Cottage"), defaultShader_, simpleShader_, textureMgr->getTexture(L"cottageD"), textureMgr->getTexture(L"CottageN"), materialLib_->getMaterial("Base")));
	sceneObjects_.back()->setObjectTransform({ 11.95,-2.1f, 66.68 }, { 0,0,0 }, { 0.2,0.2,0.2 });
	
	sceneObjects_.push_back(new Object(materialLib_->getMesh("Cottage"), defaultShader_, simpleShader_, textureMgr->getTexture(L"cottageD"), textureMgr->getTexture(L"CottageN"), materialLib_->getMaterial("Base")));
	sceneObjects_.back()->setObjectTransform({ 53.42, -0.6f, 34.97 }, { 0,90,0 }, { 0.2,0.2,0.2 });
	
	//! foliage / trees
	initFoliage();

	//! scene lights.
	addLight(LightType::Directional, { 1.f,1.f,0.6f,1.f }, { 50.f,20.f,100.f }, { 0.2f,0.2f,0.f,1.f }, { 1.f,1.f,1.f,1.f }, { 0.f,-1.f,-1.f });
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D objects.
	if (defaultShader_)
		delete defaultShader_;

	if (landscapeShader_)
		delete landscapeShader_;

	if (foliageShader_)
		delete foliageShader_;

	if (waterShader_)
		delete waterShader_;

	if (gpuOrderShader_)
		delete gpuOrderShader_;

	if (PPBlurShader_)
		delete PPBlurShader_;

	if (PPDofShader_)
		delete PPDofShader_;

	if (simpleShader_)
		delete simpleShader_;

	if (materialLib_)
		delete materialLib_;
}

bool App1::frame()
{
	float deltaTime = timer->getFPS() != 0 ? 1.f/timer->getFPS() : 0.f;

	// CAMERA UPADTAE //

	//! Generate the view matrix based on the camera's position.s
	camera->update();

	// WATER UPDATE //

	//! update time for water vertex buffer
	auto* waterData = static_cast<WaterShader::WaterParams*>(water_->getAdditionalShaderData());
	waterData->vertexBuffer.time += deltaTime * P_waterWavesSpeed;

	//! move the uv offset, moves the uvs of the water, creates the flow effect
	uvOffset = UVPanner(uvOffset, XMFLOAT2(P_waterTextureSpeed * deltaTime, -P_waterTextureSpeed * deltaTime));
	materialLib_->getMaterial("Water")->uvOffset = uvOffset;

	XMFLOAT2 newUV = UVPanner(waterData->pixelBuffer.uvOffset2, XMFLOAT2(P_waterTextureSpeed * deltaTime, 0.f));
	waterData->pixelBuffer.uvOffset2 = newUV;

	// FOLIAGE UPDATE //

	//! foliage alpha blending order, max 5000 units (can be increased with multiple dispatches)
	auto* mesh = static_cast<BetterPointMesh*>(foliage_->getMesh());
	gpuOrderShader_->Compute(renderer, mesh->getVerticesVector(), camera->getPosition());
	mesh->initBuffers(renderer->getDevice());

	// WIND UPDATE //

	//!wind update
	windParams->hullBuffer.cameraPos = camera->getPosition();
	windParams->windBuffer.uvOffset = UVPanner(windParams->windBuffer.uvOffset, XMFLOAT2(P_windSpeed * deltaTime, P_windSpeed * deltaTime));

	// LIGHT UPDATE //

	//! update the light matrices, directional light scan for much further
	lights_[0]->setDiffuseColour(P_L_dirColour.x, P_L_dirColour.y, P_L_dirColour.z, P_L_dirColour.w);
	lights_[0]->setAmbientColour(P_L_dirAmbient.x, P_L_dirAmbient.y, P_L_dirAmbient.z, P_L_dirAmbient.w);
	lights_[0]->setPosition(P_L_dirPos.x, P_L_dirPos.y, P_L_dirPos.z);
	lights_[0]->setDirection(P_L_dirDir.x, P_L_dirDir.y, P_L_dirDir.z);

	for (int i = 0; i < lights_.size(); i++)
	{
		const int sceneWidth = 200;
		const int sceneHeight = 200;
		lights_[i]->generateOrthoMatrix(sceneWidth,sceneHeight, 0.1f, lightTypes_[i] == LightType::Directional ? 100.f : 10.f);
		lights_[i]->generateViewMatrix();
	}

	bool result;
	result = BaseApplication::frame();
	if (!result)
		return false;
	
	// Render the graphics.
	result = render();
	if (!result)
		return false;

	return true;
}

bool App1::renderGeometry()
{
	//! Get the world, view and projection matrices from the camera and Direct3D objects.
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	//! render all the scene objects for the final pass
	for (auto it : sceneObjects_)
		it->render(renderer, viewMatrix, projectionMatrix, &shadowMaps_, &lights_, &lightTypes_, camera->getPosition());

	//! Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();

	return true;
}

bool App1::renderGeometryToTexture()
{
	//! create the lightmaps for All the lights, store them at the correlating index position, !!!resets to back buffer!!!
	BakeLightsMaps(renderer, shadowMaps_, lights_, sceneObjects_, camera->getOrthoViewMatrix(), wnd);

	//! Set the render target to be the render to texture and clear it
	renderTexture_->setRenderTarget(renderer->getDeviceContext());
	renderTexture_->clearRenderTarget(renderer->getDeviceContext(), P_bgColour.x, P_bgColour.y, P_bgColour.z, P_bgColour.w);

	renderGeometry();

	//! Calculate depth ito a texture (ShadowMap)
	depthTexture_->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	for (auto it : sceneObjects_)
		it->lowRender(renderer, camera->getViewMatrix(), renderer->getProjectionMatrix(), camera->getPosition());

	renderer->setBackBufferRenderTarget();

	return true;
}

bool App1::renderGeometryToBackBuffer()
{
	//! create the lightmaps for All the lights, store them at the correlating index position, !!!resets to back buffer!!!
	BakeLightsMaps(renderer, shadowMaps_, lights_, sceneObjects_, camera->getOrthoViewMatrix(), wnd);

	//! Clear the scene. (default colour)
	renderer->beginScene(P_bgColour.x, P_bgColour.y, P_bgColour.z, P_bgColour.w);

	renderGeometry();

	//! Render GUI
	gui();

	//! Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

bool App1::renderPP()
{
	// Clear the scene. (default colour)
	renderer->beginScene(P_bgColour.x, P_bgColour.y, P_bgColour.z, P_bgColour.w);

	//! Prepare matrices
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix(); 
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	

	//! get the blurred bersion of the rendered scene
	RenderTexture blurred(renderer->getDevice(), resolution_.x, resolution_.y, SCREEN_NEAR, SCREEN_DEPTH);
	BlurTexture(renderer, renderTexture_->getShaderResourceView(), PPBlurShader_, resolution_, P_DofIntensity, camera->getOrthoViewMatrix(), &blurred);

	// RENDER THE RENDER TEXTURE SCENE
	//! Requires 2D rendering and an ortho mesh.
	renderer->setZBuffer(false);

	orthoMesh_->sendData(renderer->getDeviceContext());
	PPDofShader_->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, blurred.getShaderResourceView(), renderTexture_->getShaderResourceView(), depthTexture_->getDepthMapSRV());
	PPDofShader_->render(renderer->getDeviceContext(), orthoMesh_->getIndexCount());
	renderer->setZBuffer(true);
	
	//! Render GUI
	gui();

	//! Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

bool App1::render()
{
	//! allows for DOF switching (moreso all PP effects)
	if (P_renderDof) 
	{
		//! Render first pass to render texture
		renderGeometryToTexture();

		//! Render final pass to frame buffer
		renderPP();
	}
	else 
	{
		//! standard render
		renderGeometryToBackBuffer();
	}

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	//! Access data
	auto* waterData = static_cast<WaterShader::WaterParams*>(water_->getAdditionalShaderData());
	auto* landscapeData = static_cast<LandscapeShader::LandscapeParameters*>(landscape_->getAdditionalShaderData());
	
	//! Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Camera: X %.2f, Y %.2f, Z %.2f", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Text("-Post processing and effects");
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("DOF", &P_renderDof);
	ImGui::InputFloat2("DOF Intensity", &P_DofIntensity.x, 2);
	ImGui::Text("-Water");
	ImGui::InputFloat("Wave speed", &P_waterWavesSpeed, 0.01, 0.01);
	ImGui::InputFloat("Wave texture speed", &P_waterTextureSpeed, 0.01, 0.01);
	ImGui::InputFloat2("Layer 1 - scaling", &water_->getMaterial()->uvScale.x, 2);
	ImGui::InputFloat2("Layer 2 - scaling", &waterData->pixelBuffer.uvScaling2.x, 2);
	ImGui::InputFloat("Amplitude", &waterData->vertexBuffer.waveAltitude, 0.01, 0.01);
	ImGui::InputFloat("Frequency", &waterData->vertexBuffer.waveFrequency, 0.01, 0.01);
	ImGui::Text("-Landscape");
	ImGui::InputFloat("Bottom Y", &landscapeData->bot_mid_range.first, 0.01, 0.01);
	ImGui::InputFloat("Middle Y1", &landscapeData->bot_mid_range.second, 0.01, 0.01);
	ImGui::InputFloat("Bottom Y2", &landscapeData->mid_top_range.first, 0.01, 0.01);
	ImGui::InputFloat("Top Y", &landscapeData->mid_top_range.second, 0.01, 0.01);
	ImGui::Text("-Wind");
	ImGui::InputFloat("Speed", &P_windSpeed, 0.01, 0.01);
	ImGui::Text("-Directional Light");
	ImGui::InputFloat4("Colour", &P_L_dirColour.x, 2);
	ImGui::InputFloat4("Ambient", &P_L_dirAmbient.x, 2);
	ImGui::InputFloat3("Position", &P_L_dirPos.x, 2);
	ImGui::InputFloat3("Direction", &P_L_dirDir.x, 2);
	ImGui::Text("-Background");
	ImGui::InputFloat4("Colour BG", &P_bgColour.x, 2);
	
	//! Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::addLight(LightType type, XMFLOAT4 d, XMFLOAT3 pos, XMFLOAT4 a, XMFLOAT4 s, XMFLOAT3 dir  )
{
	//! creates a new light using parameters
	lights_.push_back(new Light());
	lights_.back()->setAmbientColour( a.x, a.y, a.z, a.w );
	lights_.back()->setDiffuseColour( d.x, d.y, d.z, d.w );
	lights_.back()->setSpecularColour( s.x, s.y, s.z, s.w );
	lights_.back()->setDirection( dir.x, dir.y, dir.z);
	lights_.back()->setPosition( pos.x, pos.y, pos.z );
	lightTypes_.push_back(type);
}

void App1::initFoliage()
{
	//! responsibility for the heap struct is given to the object
	FoliageShader::FoliageParams* foliageParams = new FoliageShader::FoliageParams();
	foliageParams->scalingRangeBottom = XMFLOAT3(0.7f, 1.f, 0.7f);
	foliageParams->scalingRangeTop = XMFLOAT3(1.5f, 1.5f, 1.5f);

	sceneObjects_.push_back(new Object(new BetterPointMesh(renderer->getDevice(), renderer->getDeviceContext()), foliageShader_, simpleShader_, textureMgr->getTexture(L"tree"), NULL, materialLib_->getMaterial("Foliage"), D3D_PRIMITIVE_TOPOLOGY_POINTLIST));
	sceneObjects_.back()->setAdditionalShaderData(foliageParams);

	//! setting up foliage compute params
	FoliageShader::ComputeParams params;
	params.additionalParams.filingPercentage_GPU = 0.00005f;
	params.additionalParams.landscapeScalng = XMFLOAT3(100.f, 1, 100.f);
	params.brushMap = textureMgr->getTexture(L"foliageBrush");
	params.heightMap = textureMgr->getTexture(L"landscapeH");

	//! generates the points where each piece is based on the brush map
	foliageShader_->Compute(renderer, &params);

	//! copy the results into a separate buffer
	ID3D11Buffer* computeResult = CreateAndCopyToDebugBuf(renderer->getDevice(), renderer->getDeviceContext(), foliageShader_->_vertexComputeBuffer);
	auto* computeData = MapBufferToPointer<FoliageShader::VertexComputeBufferType[TEX_HEIGHT * TEX_WIDTH]>(renderer->getDeviceContext(), computeResult, D3D11_MAP_READ);
	int debugCount = 0;

	//! determine which points to actually write, pseudo random
	for (int i = 0; i < TEX_WIDTH * TEX_WIDTH; i++)
	{
		float r = (float)rand() / 1000.f;
		r -= std::floor(r);
		if ((*computeData)[i].position.y > -15000.f && r < fillingPercentage_CPU)
		{
			static_cast<BetterPointMesh*>(sceneObjects_.back()->getMesh())->addVertex(renderer->getDevice(), { (*computeData)[i].position, XMFLOAT2(0,0), XMFLOAT3(0,0,-1) }, { -5, -5 + 0.5 , -10 });
			debugCount++;
		}
	}
	renderer->getDeviceContext()->Unmap(computeResult, 0);
	ReleaseBuffer(&computeResult);

	//! store for later use
	foliage_ = sceneObjects_.back();
}

void App1::initWater()
{
	//! responsibility for the heap struct is given to the object
	WaterShader::WaterParams* waterP = new WaterShader::WaterParams;
	waterP->pixelBuffer.uvScaling2 = XMFLOAT2(30.f, 30.f);
	waterP->pixelBuffer.landscapeOriginPosition = sceneObjects_.back()->getPosition();
	waterP->vertexBuffer.waveAltitude = .05f;
	waterP->vertexBuffer.waveFrequency = 15;
	waterP->bottomLayer = textureMgr->getTexture(L"waterBelow");
	waterP->heightMap = textureMgr->getTexture(L"landscapeH");

	sceneObjects_.push_back(new Object(new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 100), waterShader_, simpleShader_, textureMgr->getTexture(L"water"), textureMgr->getTexture(L"stone1N"), materialLib_->getMaterial("Water")));
	water_ = sceneObjects_.back();
	water_->setObjectTransform({ -5, -3, -10 });
	water_->setAdditionalShaderData(waterP);
}

void App1::initLandscape()
{
	//! responsibility for the heap struct is given to the object
	LandscapeShader::LandscapeParameters* landscapeP = new LandscapeShader::LandscapeParameters;
	landscapeP->heightMap = textureMgr->getTexture(L"landscapeH");
	landscapeP->bottomL = textureMgr->getTexture(L"grass");
	landscapeP->middleL = textureMgr->getTexture(L"stone1");
	landscapeP->topL = textureMgr->getTexture(L"stone2");
	landscapeP->bottomL_N = textureMgr->getTexture(L"grassN");
	landscapeP->middleL_N = textureMgr->getTexture(L"stone1N");
	landscapeP->topL_N = textureMgr->getTexture(L"stone1N");
	landscapeP->bot_mid_range = { -3.0f, 1.0f };
	landscapeP->mid_top_range = { 10.f, 20.f };

	sceneObjects_.push_back(new Object(new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 100), landscapeShader_, NULL, textureMgr->getTexture(L"grass"), textureMgr->getTexture(L"landscapeN"), materialLib_->getMaterial("Land")));
	sceneObjects_.back()->setAdditionalShaderData(landscapeP);
	sceneObjects_.back()->setObjectTransform({ -5, -5, -10 });
	landscape_ = sceneObjects_.back();
}

void App1::initWind()
{
	windParams = new WindShader::WindAddititonalParams;
	windParams->hullBuffer.maxTesselationFactor = 3;
	windParams->windBuffer.intensity = 0.01;
	windParams->windBuffer.uvScale = { 1,1 };
	windParams->windTexture = textureMgr->getTexture(L"windM");
	windParams->windBrushTexture = textureMgr->getTexture(L"tree3DW");
}

