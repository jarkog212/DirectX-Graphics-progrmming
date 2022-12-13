#pragma once

#ifndef _PP_DOF_SHADER_H_
#define _PP_DOF_SHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class PPDofShader :
    public BaseShader
{
public:
	PPDofShader(ID3D11Device* device, HWND hwnd);
	~PPDofShader();

	//! follows the manual setting of the params as this is not a child of DefaultShader
	//! "depth" parameter determines the blending
	void setShaderParameters(
		ID3D11DeviceContext* deviceContext,
		const XMMATRIX& world,
		const XMMATRIX& view,
		const XMMATRIX& projection,
		ID3D11ShaderResourceView* base,
		ID3D11ShaderResourceView* blur,
		ID3D11ShaderResourceView* depth);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11SamplerState* sampleState;

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* ppBlurBuffer;
};

#endif
