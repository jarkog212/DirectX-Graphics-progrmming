#include "SimpleShader.h"
#include "ShaderUtils.h"

SimpleShader::SimpleShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	//! uses base vertex shader, the other stages are customized
	initShader(L"base_vs.cso", L"base_ps.cso");
}

SimpleShader::~SimpleShader()
{
	//! Release the sampler states.
	ReleaseSampler(&sampleState);

	//! Release the constant buffers.
	ReleaseBuffer(&matrixBuffer);

	//! Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//! Release base shader components
	BaseShader::~BaseShader();
}

void SimpleShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture)
{
	// -------- MATRIX BUFFER, vertex reg b0 ------------
	auto* dataPtr = MapBufferToPointer<MatrixBufferType>(deviceContext, matrixBuffer);
	dataPtr->world = XMMatrixTranspose(world); // worldMatrix;
	dataPtr->view = XMMatrixTranspose(view);
	dataPtr->projection = XMMatrixTranspose(projection);
	finalizeBuffer(deviceContext, matrixBuffer, Vertex, 0);

	// -------- DIFFUSE TEXTURE BUFFER, pixel reg b0 ------------
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// -------- SAMPLER BUFFER, compute reg s0 ------------
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

void SimpleShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	//! Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	//! setup sampler
	setupSampler(renderer, sampleState);

	//! setup constant buffers
	setupBuffer<MatrixBufferType>(renderer, &matrixBuffer);
}
