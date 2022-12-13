#pragma once
// Light shader.h
// Basic single light shader setup
#ifndef _DEFAULTSHADER_H_
#define _DEFAULTSHADER_H_

#define NUMOFLIGHTS 2           //current max number of lights per onbject

#include "DXF.h"
#include <limits>
#include "GlobalConstants.h"

using namespace std;
using namespace DirectX;

//! types of lights supported
enum class LightType : int
{
	PointLight = 0,
	SpotLight,
	Directional,
	Ambient
};

//!supported shading types
enum class ShadingType : int
{
	Texture_Normal = 0,
	Texture,
	Normal,
};

class DefaultShader : public BaseShader
{
public:
	//! public to allow direct creation and handling on the object level
	//! evades a parser function
	struct MaterialBufferType
	{
		XMFLOAT4 diffuse = k_InvalidFloat4;
		XMFLOAT4 emissive = k_InvalidFloat4;
		float roughness = k_NullFloat;
		float metalic = k_NullFloat;
		XMFLOAT2 uvScale = XMFLOAT2(1.f, 1.f);                      //! scalinng of the UVs, multiplies the uv coordinates
		XMFLOAT2 uvOffset = XMFLOAT2(0.f, 0.f);                     //! offset of the UVs, adds to the uv coordinaates
		float shadingType = (int)ShadingType::Texture_Normal;       //! determines whether texture or only diffuse is shown, with or without normals
		float p;
	};

protected:
	//! struct to combine the data of the Position and cutoff into a single block of float4 sized memory
	struct Position_Cutoff
	{
		XMFLOAT3 position = k_InvalidFloat3;
		float cutoff = k_InvalidFloat;
	};

	//! struct to combine the data of the direction and type into a single block of float4 sized memory
	struct Direction_Type
	{
		XMFLOAT3 direction = k_InvalidFloat3;
		float type = k_InvalidFloat;
	};

	//! buffer for transformation matrices
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	//! buffer light parameters, carries all lights in a single buffer
	struct LightBufferType
	{
		XMFLOAT4 L_diffuse[NUMOFLIGHTS];
		XMFLOAT4 L_ambient[NUMOFLIGHTS];
		Position_Cutoff L_PosCut[NUMOFLIGHTS];
		Direction_Type L_DirType[NUMOFLIGHTS];
		XMFLOAT4 L_specular[NUMOFLIGHTS];
	};

	//! buffer for transofrmation matrices for lights, used for shadows
	struct LightMatrixBufferType
	{
		XMMATRIX L_lightView[NUMOFLIGHTS];
		XMMATRIX L_lightProjection[NUMOFLIGHTS];
	};

	//! buffer for camera data
	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition = k_InvalidFloat3;
		float padding;
	};

public:
	DefaultShader(ID3D11Device* device, HWND hwnd);
	~DefaultShader();

	//! overrideable dispatch call with custom set of params
	virtual void Compute(D3D* renderer, void* computeParams) {};
	//! adds the ability to add more data into new buffers or shader stages for derived objects
	virtual void additionalParameters(ID3D11DeviceContext* device,  void* params) {};

	void setShaderParameters(
		ID3D11DeviceContext* deviceContext,
		const XMMATRIX& world,
		const XMMATRIX& view,
		const XMMATRIX& projection,
		ID3D11ShaderResourceView* texture = NULL,
		ID3D11ShaderResourceView* normalMap = NULL,
		MaterialBufferType* material = NULL,
		const std::vector<Light*>* lightArray = NULL,
		const std::vector<LightType>* lightTypes = NULL,
		const std::vector<ShadowMap*>* shadowMaps = NULL,
		XMFLOAT3 cameraPosition = k_InvalidFloat3);

protected:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	//! helper function, data from light object into the light buffer
	void LightToShaderBuffer(Light& light, LightBufferType* buffer, LightType type, int ID);

protected:
	//! buffers to be accessible to all sub shaders
	LightBufferType _lights;
	
	ID3D11Buffer* _matrixBuffer = NULL;
	ID3D11Buffer* _lightBuffer = NULL;
	ID3D11Buffer* _lightMatrixBuffer = NULL;
	ID3D11Buffer* _materialBuffer = NULL;
	ID3D11Buffer* _cameraBuffer = NULL;

	ID3D11SamplerState* _sampleState = NULL;
	ID3D11SamplerState* _sampleStateShadow = NULL;
};

#endif