// depth shader.cpp
#include "depthshader.h"
#include "ShaderUtils.h"

DepthShader::DepthShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"depth_vs.cso", L"depth_ps.cso");
}

DepthShader::~DepthShader()
{
	//! Release the matrix constant buffer.
	ReleaseBuffer(&matrixBuffer_);

	//! Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//! Release base shader components
	BaseShader::~BaseShader();
}

void DepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	//! Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	//! Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	setupBuffer<MatrixBufferType>(renderer, &matrixBuffer_);
}

void DepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix)
{	
	//! Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	//! Lock the constant buffer so it can be written to.
	auto* dataPtr = MapBufferToPointer<MatrixBufferType>(deviceContext, matrixBuffer_);
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	finalizeBuffer(deviceContext, matrixBuffer_, Vertex);
}
