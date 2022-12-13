#pragma once

#ifndef _SIMPLE_SHADER_H_
#define _SIMPLE_SHADER_H_

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class SimpleShader :
    public BaseShader
{
public:
	SimpleShader(ID3D11Device* device, HWND hwnd);
	~SimpleShader();

	//! follows the manual setting of the params as this is not a child of DefaultShader
	void setShaderParameters(
		ID3D11DeviceContext* deviceContext,
		const XMMATRIX& world,
		const XMMATRIX& view,
		const XMMATRIX& projection,
		ID3D11ShaderResourceView* texture
	);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11SamplerState* sampleState;

	ID3D11Buffer* matrixBuffer;
};

#endif