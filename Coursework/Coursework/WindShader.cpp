#include "WindShader.h"
#include "ShaderUtils.h"

WindShader::WindShader(ID3D11Device* device, HWND hwnd) : DefaultShader(device, hwnd)
{
	initShader(L"wind_vs.cso", L"default_ps.cso");
	loadHullShader(L"wind_hs.cso");
	loadDomainShader(L"wind_ds.cso");

	setupBuffer<HullBufferType>(device, &_hullBuffer);
	setupBuffer<WindBufferType>(device, &_windBuffer);
}

WindShader::~WindShader()
{
	ReleaseBuffer(&_hullBuffer);
	ReleaseBuffer(&_windBuffer);

	DefaultShader::~DefaultShader();
}

void WindShader::additionalParameters(ID3D11DeviceContext* device, void* params)
{
	device->DSSetConstantBuffers(0, 1, &_matrixBuffer);
	device->DSSetConstantBuffers(1, 1, &_cameraBuffer);
	device->DSSetConstantBuffers(2, 1, &_lightMatrixBuffer);

	auto* data = static_cast<WindAddititonalParams*>(params);
	
	auto* bufferPtr = MapBufferToPointer<HullBufferType>(device, _hullBuffer);
	memcpy(bufferPtr, &data->hullBuffer, sizeof(HullBufferType));
	finalizeBuffer(device, _hullBuffer, Hull, 0);

	auto* windPtr = MapBufferToPointer<WindBufferType>(device, _windBuffer);
	memcpy(windPtr, &data->windBuffer, sizeof(WindBufferType));
	finalizeBuffer(device, _windBuffer, Domain, 3);

	device->DSSetShaderResources(0, 1, &data->windTexture);
	device->DSSetShaderResources(1, 1, &data->windBrushTexture);

	device->DSSetSamplers(0, 1, &sampleState_);
}