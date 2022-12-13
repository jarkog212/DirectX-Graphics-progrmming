#include "FoliageShader.h"
#include "ShaderUtils.h"

FoliageShader::FoliageShader(ID3D11Device* device, HWND hwnd) : DefaultShader(device, hwnd)
{
	//! uses defalt pixel shader, the other stages are ccustomized
	initShader(L"foliage_vs.cso", L"default_ps.cso");
	loadGeometryShader(L"foliage_gs.cso");
	loadComputeShader(L"foliage_cs.cso");

	//! create buffers
	CreateStructuredBuffer(device, sizeof(VertexComputeBufferType), TEX_HEIGHT * TEX_WIDTH, nullptr, &_vertexComputeBuffer);
	setupBuffer<ConstantComputeBufferType>(renderer, &_constantComputeBuffer);
	setupBuffer<TransformBufferType>(renderer, &_transformBuffer);
}

FoliageShader::~FoliageShader()
{
	//! cleanup new buffers
	ReleaseBuffer(&_constantComputeBuffer);
	ReleaseBuffer(&_vertexComputeBuffer);
	ReleaseBuffer(&_transformBuffer);

	//! cleanup inherited objects
	DefaultShader::~DefaultShader();
}

//! separate compute call
void FoliageShader::Compute(D3D* renderer, void* computeParams)
{
	//! cast to an appropriate memory layout
	auto data = static_cast<ComputeParams*>(computeParams);

	//! setup compute buffers, done individually since not a part of the render call
	
	// -------- CONSTANT COMPUTE BUFFER, compute reg b0 ------------
	
	auto* computePtr = MapBufferToPointer<ConstantComputeBufferType>(renderer->getDeviceContext(), _constantComputeBuffer);
	memcpy(computePtr, &data->additionalParams, sizeof(ConstantComputeBufferType));
	finalizeBuffer(renderer->getDeviceContext(), _constantComputeBuffer, PipelineStage::Compute);

	// -------- VERTEX COMPUTE BUFFER, compute reg u0 ------------
	//! UAV only buffer, uses external code
    ID3D11UnorderedAccessView* UAV;
	CreateBufferUAV(renderer->getDevice(),_vertexComputeBuffer, &UAV);
	renderer->getDeviceContext()->CSSetUnorderedAccessViews(0, 1, &UAV, nullptr);

	// -------- SAMPLER BUFFER, compute reg s0 ------------
	//! Sampler, predefined in default shader, originally for pixel shader
	renderer->getDeviceContext()->CSSetSamplers(0, 1, &_sampleState);
	
	//! Textures to be used by the computing
	// -------- HEIGHT MAP BUFFER, compute reg t0 ------------
	renderer->getDeviceContext()->CSSetShaderResources(0, 1, &data->heightMap);

	// -------- BRUSH MAP BUFFER, compute reg t1 ------------
	renderer->getDeviceContext()->CSSetShaderResources(1, 1, &data->brushMap);

	//! dispatch call, create 1024 groups of 1024 threads each
	compute(renderer->getDeviceContext(), 1024, 1, 1);
}

//! part of the render process, specifies how to handle additional parameters
void FoliageShader::additionalParameters(ID3D11DeviceContext* device, void* params)
{
	//! reuse matrix and camera buffers in geometry stage, originaly used for vertex shader
	//! geo reg b0-b1
	device->GSSetConstantBuffers(0, 1, &_matrixBuffer);
	device->GSSetConstantBuffers(1, 1, &_cameraBuffer);

	// -------- FOLIAGE PARAMETERS BUFFER, geo reg b2 ------------
	//! casts the input params back to usable state
	auto* data = static_cast<FoliageParams*>(params);
	auto* transformPtr = MapBufferToPointer<TransformBufferType>(device, _transformBuffer);
	transformPtr->scalingRangeBottom = data->scalingRangeBottom;
	transformPtr->scalingRangeTop = data->scalingRangeTop;
	finalizeBuffer(device, _transformBuffer, Geo, 2);
}
