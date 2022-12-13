#include "WaterShader.h"
#include "ShaderUtils.h"

WaterShader::WaterShader(ID3D11Device* device, HWND hwnd) : DefaultShader(device, hwnd)
{
	initShader(L"water_vs.cso", L"water_ps.cso");

	setupBuffer<WaterPixelBufferType>(renderer, &_waterBuffer);
	setupBuffer<WaterVertexBufferType>(renderer, &_waveBuffer);
}

WaterShader::~WaterShader()
{
	ReleaseBuffer(&_waterBuffer);
	ReleaseBuffer(&_waveBuffer);

	DefaultShader::~DefaultShader();
}

void WaterShader::additionalParameters(ID3D11DeviceContext* device, void* params)
{
	auto* data = static_cast<WaterParams*>(params);

	auto* waterPtr = MapBufferToPointer<WaterPixelBufferType>(device, _waterBuffer);
	memcpy(waterPtr, &data->pixelBuffer, sizeof(WaterPixelBufferType));
	finalizeBuffer(device, _waterBuffer, Pixel, 2);

	auto* VwaterPtr = MapBufferToPointer<WaterVertexBufferType>(device, _waveBuffer);
	memcpy(VwaterPtr, &data->vertexBuffer, sizeof(WaterVertexBufferType));
	finalizeBuffer(device, _waveBuffer, Vertex, 3);

	device->PSSetShaderResources(10, 1, &data->bottomLayer);
	device->PSSetShaderResources(11, 1, &data->heightMap);
}
