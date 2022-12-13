#include "PPBoxShader.h"
#include "ShaderUtils.h"

PPBoxShader::PPBoxShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	//! uses base vertex shader, the other stages are customized
	initShader(L"base_vs.cso", L"ppbox_ps.cso");
}

PPBoxShader::~PPBoxShader()
{
	//! Release the sampler states.
	ReleaseSampler(&sampleState);

	//! Release the constant buffers.
	ReleaseBuffer(&matrixBuffer);
	ReleaseBuffer(&ppBoxBuffer);

	//! Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//! Release base shader components
	BaseShader::~BaseShader();
}


void PPBoxShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	//! Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//! setup sampler
	setupSampler(renderer, sampleState);

	//! setup constant buffers
	setupBuffer<MatrixBufferType>(renderer, &matrixBuffer);
	setupBuffer<PPBoxBufferType>(renderer, &ppBoxBuffer);
}


void PPBoxShader::setShaderParameters(
	ID3D11DeviceContext* deviceContext, 
	const XMMATRIX& worldMatrix, 
	const XMMATRIX& viewMatrix, 
	const XMMATRIX& projectionMatrix, 
	ID3D11ShaderResourceView* texture, 
	XMFLOAT2 resolution, 
	XMFLOAT3X3 kernel, 
	float cummulativeWeight)
{
	// -------- MATRIX BUFFER, vertex reg b0 ------------
	auto* dataPtr = MapBufferToPointer<MatrixBufferType>(deviceContext, matrixBuffer);
	dataPtr->world = XMMatrixTranspose(worldMatrix); // worldMatrix;
	dataPtr->view = XMMatrixTranspose(viewMatrix);
	dataPtr->projection = XMMatrixTranspose(projectionMatrix);
	finalizeBuffer(deviceContext, matrixBuffer, Vertex, 0);

	// -------- POST PROCESS BUFFER, pixel reg b0 ------------
	auto* ppBoxPtr = MapBufferToPointer<PPBoxBufferType>(deviceContext, ppBoxBuffer);
	ppBoxPtr->resolution = resolution;
	ppBoxPtr->rAbove = XMFLOAT3(kernel._11, kernel._12, kernel._13);
	ppBoxPtr->rCurrent = XMFLOAT3(kernel._21, kernel._22, kernel._23);
	ppBoxPtr->rBelow = XMFLOAT3(kernel._31, kernel._32, kernel._33);
	ppBoxPtr->cummulativeWeight = cummulativeWeight;
	finalizeBuffer(deviceContext, ppBoxBuffer, Pixel, 0);

	//! Set shader texture and sampler resource in the pixel shader.
	// -------- RENDER TEXTURE BUFFER, pixel reg t0 ------------
	deviceContext->PSSetShaderResources(0, 1, &texture);

	// -------- SAMPLER BUFFER, pixel reg s0 ------------
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}





