#include "PPBlurShader.h"
#include "ShaderUtils.h"

PPBlurShader::PPBlurShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	//! uses base vertex shader, the other stages are customized
	initShader(L"base_vs.cso", L"ppblur_ps.cso");
}

PPBlurShader::~PPBlurShader()
{
	//! Release the sampler states.
	ReleaseSampler(&sampleState);

	//! Release the constant buffers.
	ReleaseBuffer(&matrixBuffer);
	ReleaseBuffer(&ppBlurBuffer);

	//! Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//! Release base shader components
	BaseShader::~BaseShader();
}

void PPBlurShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, XMFLOAT2 blurDimensions, XMFLOAT2 resolution)
{
	// -------- MATRIX BUFFER, vertex reg b0 ------------
	auto* dataPtr = MapBufferToPointer<MatrixBufferType>(deviceContext, matrixBuffer);
	dataPtr->world = XMMatrixTranspose(world); // worldMatrix;
	dataPtr->view = XMMatrixTranspose(view);
	dataPtr->projection = XMMatrixTranspose(projection);
	finalizeBuffer(deviceContext, matrixBuffer, Vertex, 0);

	// -------- POST PROCESS BLUR BUFFER, pixel reg b0 ------------
	auto* ppBlurPtr = MapBufferToPointer<PPBlurBufferType>(deviceContext, ppBlurBuffer);
	ppBlurPtr->resolution = resolution;
	ppBlurPtr->dimensions = blurDimensions;
	finalizeBuffer(deviceContext, ppBlurBuffer, Pixel, 0);

	//! Set shader texture and sampler resource in the pixel shader.
	// -------- RENDER TEXTURE BUFFER, pixel reg t0 ------------
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// -------- SAMPLER BUFFER, pixel reg s0 ------------
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

void PPBlurShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	//! Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	//! setup sampler
	auto m = D3D11_TEXTURE_ADDRESS_BORDER;
	setupSampler(renderer, sampleState,m,m,m,1,1,1,1);

	//! setup constant buffers
	setupBuffer<MatrixBufferType>(renderer, &matrixBuffer);
	setupBuffer<PPBlurBufferType>(renderer, &ppBlurBuffer);
}
