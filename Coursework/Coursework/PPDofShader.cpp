#include "PPDofShader.h"
#include "ShaderUtils.h"

PPDofShader::PPDofShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	//! uses base vertex shader, the other stages are customized
	initShader(L"base_vs.cso", L"ppdof_ps.cso");
}

PPDofShader::~PPDofShader()
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

void PPDofShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* base, ID3D11ShaderResourceView* blur, ID3D11ShaderResourceView* depth)
{
	// -------- MATRIX BUFFER, vertex reg b0 ------------
	auto* dataPtr = MapBufferToPointer<MatrixBufferType>(deviceContext, matrixBuffer);
	dataPtr->world = XMMatrixTranspose(world); // worldMatrix;
	dataPtr->view = XMMatrixTranspose(view);
	dataPtr->projection = XMMatrixTranspose(projection);
	finalizeBuffer(deviceContext, matrixBuffer, Vertex, 0);

	// -------- BLUR RENDER TEXTURE BUFFER, pixel reg t0 ------------
	deviceContext->PSSetShaderResources(0, 1, &blur);

	// -------- BASE RENDER TEXTURE BUFFER, pixel reg t1 ------------
	deviceContext->PSSetShaderResources(1, 1, &base);

	// -------- DEPTH MAP TEXTURE BUFFER, pixel reg t2 ------------
	deviceContext->PSSetShaderResources(2, 1, &depth);

	// -------- SAMPLER BUFFER, compute reg s0 ------------
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

void PPDofShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	//! Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	//! setup sampler
	auto m = D3D11_TEXTURE_ADDRESS_BORDER;
	setupSampler(renderer, sampleState, m, m, m, 1, 1, 1, 1);

	//! setup constant buffers
	setupBuffer<MatrixBufferType>(renderer, &matrixBuffer);
}
