#include "Object.h"

Object::Object(BaseMesh* mesh, DefaultShader* defaultShader_, SimpleShader* simpleShader, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* normalMap, DefaultShader::MaterialBufferType* material, D3D_PRIMITIVE_TOPOLOGY top) :
	_mesh(mesh),
	_shader(defaultShader_),
	_lowShader(simpleShader),
	_texture(texture),
	_normalMap(normalMap),
	_material(material),
	_top(top)
{
}

//! setter for transform data, uses default values to ignore aspects that are not changed
void Object::setObjectTransform(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
{
	if (!isnan(pos.x))
		_position = pos;

	if (!isnan(rot.x))
		_rotation = rot;

	if (!isnan(scale.x))
		_scale = scale;
}

//! generates the transform matrix used for proper world placement based on the kept transofrm data
void Object::applyTransform(XMMATRIX& world)
{
	//! rotation
	world = XMMatrixMultiply(world, XMMatrixRotationX(_rotation.x));
	world = XMMatrixMultiply(world, XMMatrixRotationY(_rotation.y));
	world = XMMatrixMultiply(world, XMMatrixRotationZ(_rotation.z));

	//! scale
	XMMATRIX scaleMatrix = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
	world = XMMatrixMultiply(world, scaleMatrix);

	//! position
	world = XMMatrixMultiply(world, XMMatrixTranslation(_position.x, _position.y, _position.z));
}

void Object::render(
	D3D* renderer, 
	XMMATRIX viewMatrix,
	XMMATRIX perspectiveMatrix,
	const std::vector<ShadowMap*>* shadowMaps,
	const std::vector<Light*>* lightArray,
	const std::vector<LightType>* lightTypes,
	XMFLOAT3 cameraPos
	)
{
	//! apply transform
	auto worldMatrix = renderer->getWorldMatrix();
	applyTransform(worldMatrix);

	//! send and setup data
	_mesh->sendData(renderer->getDeviceContext(),_top);
	_shader->setShaderParameters(
		renderer->getDeviceContext(), 
		worldMatrix, 
		viewMatrix, 
		perspectiveMatrix,
		_texture,
		_normalMap,
		_material,
		lightArray,
		lightTypes,
		shadowMaps,
		cameraPos
		);
	
	//! setup futher parameters, for derived shaders
	_shader->additionalParameters(renderer->getDeviceContext(), _additionalShaderData);
	
	//! if transparency enabled, ensure rendering happens with it
	if (_material->diffuse.w < 1.f)
		renderer->setAlphaBlending(true);
	
	//! render/ draw call to the GPU
	_shader->render(renderer->getDeviceContext(), _mesh->getIndexCount());

	//! clean up transparency 
	renderer->setAlphaBlending(false);
}

//! cheaper render, subject to unavaliability if simple shader is not provideds
void Object::lowRender(D3D* renderer, XMMATRIX viewMatrix, XMMATRIX perspectiveMatrix, XMFLOAT3 cameraPos)
{
	if (!_lowShader) 
	{
		render(renderer, viewMatrix, perspectiveMatrix, NULL, NULL, NULL, cameraPos);
		return;
	}

	//! apply transform
	auto worldMatrix = renderer->getWorldMatrix();
	applyTransform(worldMatrix);

	//! send and render data
	_mesh->sendData(renderer->getDeviceContext(), _top);
	_lowShader->setShaderParameters(
		renderer->getDeviceContext(),
		worldMatrix,
		viewMatrix,
		perspectiveMatrix,
		_texture);

	//! render/ draw call to the GPU
	_lowShader->render(renderer->getDeviceContext(), _mesh->getIndexCount());
}
