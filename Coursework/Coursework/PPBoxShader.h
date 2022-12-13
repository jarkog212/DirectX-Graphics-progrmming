#pragma once

#ifndef _PP_BOX_SHADER_H_
#define _PP_BOX_SHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class PPBoxShader : public BaseShader
{
private:
	//! PP buffer require these for proper texel and positinoal information
	//! also send kernel as 3 separate arrays, done to simplify shader code
	struct PPBoxBufferType 
	{
		XMFLOAT2 resolution;
		XMFLOAT2 padding;
		XMFLOAT3 rAbove;
		float padding2;
		XMFLOAT3 rCurrent; //! 'y' is the current pixel
		float padding3;
		XMFLOAT3 rBelow;
		float cummulativeWeight;
	};

public:
	PPBoxShader(ID3D11Device* device, HWND hwnd);
	~PPBoxShader();

	//! follows the manual setting of the params as this is not a child of DefaultShader
	void setShaderParameters(
		ID3D11DeviceContext* deviceContext, 
		const XMMATRIX& world, 
		const XMMATRIX& view, 
		const XMMATRIX& projection, 
		ID3D11ShaderResourceView* texture, 
		XMFLOAT2 resolution, 
		XMFLOAT3X3 kernel, 
		float cummulativeWeight);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11SamplerState* sampleState;

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* ppBoxBuffer;
};

#endif