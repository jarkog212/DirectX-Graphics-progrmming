#pragma once
#ifndef _SHADERUTILS_H_
#define _SHADERUTILS_H_

#include "GlobalConstants.h"
#include "DXF.h"	// include dxframework

using namespace DirectX;

//! forward declaration for pointer type, essentially a promise
class Object;
class PPBlurShader;

//! enum used as a way to distinguish to which shader stage to send a buffer to
enum PipelineStage : int
{
	Vertex = 0,
	Pixel,
	Geo,
	Domain,
	Hull,
	Compute
};

// FUNCTIONS //

//SETUP BUFFER FUNCTION-----------------------------------------------------------------------
//! uses a set of predefined params used most commonly by the program to properly setup a buffer
//! uses default values so the different param combos are supported if needed
//! BufferType determines the struct to use for the size and length
template<typename BufferType>
void setupBuffer(
	ID3D11Device* renderer,
	ID3D11Buffer** buffer,
	D3D11_USAGE usage = D3D11_USAGE_DYNAMIC,
	int bindFlags = D3D11_BIND_CONSTANT_BUFFER,
	int accessFlags = D3D11_CPU_ACCESS_WRITE,
	int miscFlags = 0,
	int stride = 0)
{
	D3D11_BUFFER_DESC outBufferDesc;

	outBufferDesc.Usage = usage;
	outBufferDesc.ByteWidth = sizeof(BufferType);
	outBufferDesc.BindFlags = bindFlags;
	outBufferDesc.CPUAccessFlags = accessFlags;
	outBufferDesc.MiscFlags = miscFlags;
	outBufferDesc.StructureByteStride = stride;

	renderer->CreateBuffer(&outBufferDesc, NULL, buffer);
}

//MAP BUFFER TO POINTER FUNCTIONS------------------------------------------------------------------------
//! uses a set of predefined params to allow access to a buffer. Uses default definitions hence can be modified if needed
//! since the T type pointer is returned, T type needs to match the original setup type used, this is up to the developer to uphold
template<typename T>
T* MapBufferToPointer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, D3D11_MAP mapping = D3D11_MAP_WRITE_DISCARD, int subID = 0)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result;
	result = context->Map(buffer, subID, mapping, 0, &mappedResource);
	return static_cast<T*>(mappedResource.pData);
}

//!alternative where the mapped resource is exposed and can be added externally, not used
template<typename T>
T* MapBufferToPointer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, D3D11_MAPPED_SUBRESOURCE &mappedResource, D3D11_MAP mapping = D3D11_MAP_WRITE_DISCARD)
{
	HRESULT result;
	result = context->Map(buffer, 0, mapping, 0, &mappedResource);
	return static_cast<T*>(mappedResource.pData);
}

//SETUP SAMPLER FUNCTION -----------------------------------------------------------------------
//! uses a set of predefined params used most commonly by the program to properly setup a sampler
//! uses default values so the different param combos are supported if needed
void setupSampler(
	ID3D11Device* renderer,
	ID3D11SamplerState* sampleState_,
	D3D11_TEXTURE_ADDRESS_MODE modeU = D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_MODE modeV = D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_MODE modeW = D3D11_TEXTURE_ADDRESS_CLAMP,
	float borderColor_R = 0.f,
	float borderColor_G = 0.f,
	float borderColor_B = 0.f,
	float borderColor_A = 0.f,
	D3D11_FILTER filter = D3D11_FILTER_ANISOTROPIC,
	float MipLoadBias = 0.f,
	int MaxAnisotropy = 1,
	D3D11_COMPARISON_FUNC cmpFunc = D3D11_COMPARISON_ALWAYS,
	float MinLOD = 0,
	float MaxLOD = D3D11_FLOAT32_MAX);

//FINALIZE BUFFER FUNCTION --------------------------------------------------------------------
//! unmaps the buffer and send it to the correct register in the correct stage
void finalizeBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, PipelineStage stage, int bufferID = 0);

//RELEASE FUNCTIONS -------------------------------------------------------------------------
//! safely relase the memory dedicated to the respective DX11 objects
void ReleaseBuffer(ID3D11Buffer** buffer);
void ReleaseSampler(ID3D11SamplerState** sampler);

//BAKE LIGHT MAPS FUNCTION ------------------------------------------------------------------
//! defines, rnders and stores correctly the shadow maps for each light
void BakeLightsMaps(D3D* renderer, std::vector<ShadowMap*>& maps, std::vector<Light*>& lights, std::vector<Object*>& sceneObjects, XMMATRIX& view, HWND win);

//BLUR TEXTURE FUNCTION--------------------------------------------------------------------------
//! takes in the texture and the shader used to blur it and stores the result in the specified object
void BlurTexture(D3D* renderer, ID3D11ShaderResourceView* toBlur, PPBlurShader* shader, XMFLOAT2 resolution, XMFLOAT2 blurDimensions, XMMATRIX& view, RenderTexture* result);

//UV PANNER FUNCTION
//! updates the uvOffset paraeter based on time and speed-------------------------------------------
XMFLOAT2 UVPanner(XMFLOAT2 currentOffsetUV, XMFLOAT2 speed);

//! Eternal code
HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut);
HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut);
ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer);

#endif