#include "LandscapeShader.h"
#include "ShaderUtils.h"

LandscapeShader::LandscapeShader(ID3D11Device* device, HWND hwnd) : DefaultShader(device, hwnd)
{
	//! loads the specialised, sub shaders
	initShader(L"landscape_vs.cso", L"landscape_ps.cso");

	//! create buffers
	setupBuffer<LandscapeBufferType>(renderer, &_landscapeBuffer);
}

LandscapeShader::~LandscapeShader()
{
	//! cleanup new buffers
	ReleaseBuffer(&_landscapeBuffer);

	//! cleanup inherited objects
	DefaultShader::~DefaultShader();
}

//! part of the render process, specifies how to handle additional parameters
void LandscapeShader::additionalParameters(ID3D11DeviceContext* device, void* params)
{
	//! casts the input params back to usable state
	auto data = static_cast<LandscapeParameters*>(params);
	
	// -------- LADNSCAPE BUFFER, pixel reg b2 ------------
	auto* landscapePtr = MapBufferToPointer<LandscapeBufferType>(device, _landscapeBuffer);
	landscapePtr->ranges.x = data->bot_mid_range.first;
	landscapePtr->ranges.y = data->bot_mid_range.second;
	landscapePtr->ranges.z = data->mid_top_range.first;
	landscapePtr->ranges.w = data->mid_top_range.second;
	finalizeBuffer(device, _landscapeBuffer, Pixel, 2);
	
	//! put the textures in arrays for easier handling
	ID3D11ShaderResourceView* layersTex[3] =
	{
		data->bottomL,
		data->middleL,
		data->topL
	};

	ID3D11ShaderResourceView* layersNormals[3] =
	{
		data->bottomL_N,
		data->middleL_N,
		data->topL_N
	};

	// -------- DIFFUSE TEXTURES BUFFER, pixel reg t10-t12 ------------
	device->PSSetShaderResources(10, 3, layersTex);     //pending change based on max number of lights
	
	// -------- NORMAL MAPS BUFFER, pixel reg t13-t15 ------------
	device->PSSetShaderResources(13, 3, layersNormals); //pending change based on max number of lights

	// -------- HEIGHT MAP BUFFER, vertex reg t0 ------------
	device->VSSetShaderResources(0, 1, &data->heightMap);

	// -------- SAMPLER BUFFER, compute reg s0 ------------
	device->VSSetSamplers(0, 1, &_sampleState);
}