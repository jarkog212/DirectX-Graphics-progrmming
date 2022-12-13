#pragma once
#ifndef _OBJECT_H
#define _OBJECT_H
#include "DXF.h"	// include dxframework
#include "GlobalConstants.h"
#include "DefaultShader.h"
#include "SimpleShader.h"

class Object
{
	//! base object attributes
	BaseMesh* _mesh;
	DefaultShader* _shader;
	SimpleShader* _lowShader;
	void* _additionalShaderData;     //! additional data is stored within object who propagates it to the shader with a funciton call, type needs to match shaders additional params
	XMFLOAT3 _position = { 0,0,0 };
	XMFLOAT3 _rotation = { 0,0,0 };
	XMFLOAT3 _scale = { 1,1,1 };
	ID3D11ShaderResourceView* _texture = NULL;
	ID3D11ShaderResourceView* _normalMap = NULL;
	DefaultShader::MaterialBufferType* _material = NULL;
	D3D_PRIMITIVE_TOPOLOGY _top = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bool ownerOfAdittionalParams = true;

public:
	Object(
		BaseMesh* mesh,
		DefaultShader* _shader,
		SimpleShader* _lowShader = NULL,
		ID3D11ShaderResourceView* _texture = NULL,
		ID3D11ShaderResourceView* _normalMap = NULL,
		DefaultShader::MaterialBufferType* _material = NULL,
		D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	~Object() { if (_additionalShaderData && ownerOfAdittionalParams) delete _additionalShaderData; };

	//! accessors
	void setObjectTransform(XMFLOAT3 pos = k_InvalidFloat3, XMFLOAT3 rot = k_InvalidFloat3, XMFLOAT3 scale = k_InvalidFloat3);
	void setAdditionalShaderData(void* data, bool owner = true) { _additionalShaderData = data; ownerOfAdittionalParams = owner; };
	void* getAdditionalShaderData() { return _additionalShaderData; }

	XMFLOAT3 getPosition() { return _position; }
	XMFLOAT3 getRotation() { return _rotation; }
	XMFLOAT3 getScale() { return _scale; }
	BaseMesh* getMesh() { return _mesh; }
	DefaultShader::MaterialBufferType* getMaterial() { return _material; }

	//! concatenates the matrices
	void applyTransform(XMMATRIX& world);

	//! calls the appropriate shader functions to result in a correct render procedure
	void render(
		D3D* renderer,
		XMMATRIX viewMatrix,
		XMMATRIX perspectiveMatrix,
		const std::vector<ShadowMap*>* shadowMaps = NULL,
		const std::vector<Light*>* lightArray = NULL,
		const std::vector<LightType>* lightTypes = NULL,
		XMFLOAT3 cameraPos = { 0,0,0 }
		);
	//! used for depth maps or in any other case where it suits the situation
	//! if no simple shader is present, simply calls standard render.
	void lowRender(
		D3D* renderer,
		XMMATRIX viewMatrix,
		XMMATRIX perspectiveMatrix,
		XMFLOAT3 cameraPos
		);
};

#endif
