#pragma once

#ifndef _PP_BLUR_SHADER_H_
#define _PP_BLUR_SHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class PPBlurShader :
    public BaseShader
{
private:
	//! PP shaders require these for proper texel and positinoal information
	struct PPBlurBufferType
	{
		XMFLOAT2 resolution;
		XMFLOAT2 dimensions;
	};

public:
	PPBlurShader(ID3D11Device* device, HWND hwnd);
	~PPBlurShader();

	//! follows the manual setting of the params as this is not a child of DefaultShader
	void setShaderParameters(
		ID3D11DeviceContext* deviceContext,
		const XMMATRIX& world,
		const XMMATRIX& view,
		const XMMATRIX& projection,
		ID3D11ShaderResourceView* texture,
		XMFLOAT2 blurDimensions,
		XMFLOAT2 resolution);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11SamplerState* sampleState;

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* ppBlurBuffer;
};

#endif