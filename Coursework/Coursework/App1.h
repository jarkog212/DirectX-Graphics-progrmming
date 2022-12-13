// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "Object.h"
#include "MaterialLibrary.h"
#include "LandscapeShader.h"
#include "FoliageShader.h"
#include "WaterShader.h"
#include "GPUOrderShader.h"
#include "WindShader.h"
#include "PPBlurShader.h"
#include "PPDofShader.h"
#include "SimpleShader.h"

enum class LightType : int;

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	//! separate renders to allow rendering with and without Post processing
	bool renderGeometry();
	bool renderGeometryToTexture();
	bool renderGeometryToBackBuffer();
	bool renderPP();
	bool render();
	void gui();

	//!helper function to add lights
	void addLight(
		LightType type,
		XMFLOAT4 diffuse,
		XMFLOAT3 position,
		XMFLOAT4 ambient = { 0.f,0.f,0.f,0.f },
		XMFLOAT4 specular = { 1.f,1.f,1.f,1.f },
		XMFLOAT3 direction = { 0.f,-1.f,0.f }
	);

private:
	//! separate initialisers of different aspects
	void initFoliage();
	void initWater();
	void initLandscape();
	void initWind();

	const float fillingPercentage_CPU = 0.5f;

	XMFLOAT2 uvOffset = XMFLOAT2(0.f, 0.f);

	std::vector<Object*> sceneObjects_;
	std::vector<ShadowMap*> shadowMaps_;
	std::vector<Light*> lights_;
	std::vector<LightType> lightTypes_;

	//! Shaders
	DefaultShader* defaultShader_ = NULL;
	LandscapeShader* landscapeShader_ = NULL;
	FoliageShader* foliageShader_ = NULL;
	WaterShader* waterShader_ = NULL;
	GPUOrderShader* gpuOrderShader_ = NULL;
	WindShader* windShader_ = NULL;
	PPBlurShader* PPBlurShader_ = NULL;
	PPDofShader* PPDofShader_ = NULL;
	SimpleShader* simpleShader_ = NULL;

	MaterialLibrary* materialLib_ = NULL;
	RenderTexture* renderTexture_;
	ShadowMap* depthTexture_;
	OrthoMesh* orthoMesh_;
	XMFLOAT2 resolution_;

	//! deleted as a part of scene objects vector deletion
	Object* water_ = NULL;
	Object* foliage_ = NULL;
	Object* landscape_ = NULL;
	WindShader::WindAddititonalParams* windParams = NULL;

	//!settable params
	bool P_renderDof = false;
	float P_waterWavesSpeed = 5.f;
	float P_waterTextureSpeed = 0.005;
	float P_windSpeed = 0.2f;
	XMFLOAT3 P_L_dirPos = { 50.f,20.f,100.f };
	XMFLOAT3 P_L_dirDir = { 0.f,-1.f,-1.f };
	XMFLOAT4 P_L_dirColour = { 1.f,1.f,0.6f,1.f };
	XMFLOAT4 P_L_dirAmbient = { 0.2f,0.2f,0.f,1.f };
	XMFLOAT4 P_bgColour = { 0.39f, 0.39f, 0.39f, 1.0f };
	XMFLOAT2 P_DofIntensity = { 3,3 };
};

#endif