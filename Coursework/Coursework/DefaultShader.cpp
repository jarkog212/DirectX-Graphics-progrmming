#include "DefaultShader.h"
#include "ShaderUtils.h"

DefaultShader::DefaultShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"default_vs.cso", L"default_ps.cso");
}

DefaultShader::~DefaultShader()
{
	ReleaseSampler(&_sampleState);
	ReleaseSampler(&_sampleStateShadow);
	
	ReleaseBuffer(&_matrixBuffer);
	ReleaseBuffer(&_lightBuffer);
	ReleaseBuffer(&_lightMatrixBuffer);
	ReleaseBuffer(&_materialBuffer);
	ReleaseBuffer(&_cameraBuffer);

	BaseShader::~BaseShader();
}

void DefaultShader::setShaderParameters(
	ID3D11DeviceContext* deviceContext,
	const XMMATRIX& world,
	const XMMATRIX& view,
	const XMMATRIX& projection,
	ID3D11ShaderResourceView* texture,
	ID3D11ShaderResourceView* normalMap,
	MaterialBufferType* material,
	const std::vector<Light*>* lightArray,
	const std::vector<LightType>* lightTypes,
	const std::vector<ShadowMap*>* shadowMaps,
	XMFLOAT3 cameraPosition)
{
// -------- MATRIX BUFFER, vertex reg b0 ------------

	XMMATRIX tworld = XMMatrixTranspose(world);
	XMMATRIX tview = XMMatrixTranspose(view);
	XMMATRIX tproj = XMMatrixTranspose(projection);

	auto* dataPtr = MapBufferToPointer<MatrixBufferType>(deviceContext, _matrixBuffer);
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	finalizeBuffer(deviceContext, _matrixBuffer, Vertex, 0);

// -------- CAMERA BUFFER, vertex reg b1 ------------

	auto* camPtr = MapBufferToPointer<CameraBufferType>(deviceContext, _cameraBuffer);
	camPtr->cameraPosition = cameraPosition;
	finalizeBuffer(deviceContext, _cameraBuffer, Vertex, 1);

// -------- LIGHT MATRIX BUFFER, vetrex reg b2 ------------
	
	XMMATRIX tLightViewMatrix[NUMOFLIGHTS];
	XMMATRIX tLightProjectionMatrix[NUMOFLIGHTS];
	XMFLOAT4 direction[NUMOFLIGHTS];
	
	//! iterate and fill the buffer with all lights, might need modification of more light or light gathering is to be implemented
	for (int i = 0; i < NUMOFLIGHTS; i++) {
		if (lightArray && i < lightArray->size()) {
			tLightViewMatrix[i] = XMMatrixTranspose((*lightArray)[i]->getViewMatrix());
			tLightProjectionMatrix[i] = XMMatrixTranspose((*lightArray)[i]->getOrthoMatrix());
			direction[i].x = (*lightArray)[i]->getDirection().x;
			direction[i].y = (*lightArray)[i]->getDirection().y;
			direction[i].z = (*lightArray)[i]->getDirection().z;
		}
		else
			direction[i] = k_InvalidFloat4;
	}

	auto* lightMPtr = MapBufferToPointer<LightMatrixBufferType>(deviceContext, _lightMatrixBuffer);
	memcpy(&lightMPtr->L_lightView, &tLightViewMatrix, sizeof(XMMATRIX) * NUMOFLIGHTS);
	memcpy(&lightMPtr->L_lightProjection, &tLightProjectionMatrix, sizeof(XMMATRIX) * NUMOFLIGHTS);
	finalizeBuffer(deviceContext, _lightMatrixBuffer, Vertex, 2);

// -------- MATERIAL BUFFER, pixel reg b0 ------------

	auto materialPtr = MapBufferToPointer<MaterialBufferType>(deviceContext, _materialBuffer);
	if (material)
		*materialPtr = *material;
	finalizeBuffer(deviceContext, _materialBuffer, Pixel, 0);

// -------- LIGHT BUFFER, pixel reg b1 ------------

	auto* lightPtr = MapBufferToPointer<LightBufferType>(deviceContext, _lightBuffer);
	for (int i = 0; i < NUMOFLIGHTS; i++)
	{
		if (lightArray && i < lightArray->size())
			LightToShaderBuffer(*(*lightArray)[i], lightPtr, (*lightTypes)[i], i);
		else
		{
			lightPtr->L_ambient[i] = k_InvalidFloat4;
			lightPtr->L_diffuse[i] = k_InvalidFloat4;
			lightPtr->L_specular[i] = k_InvalidFloat4;
		}
	}
	finalizeBuffer(deviceContext, _lightBuffer, Pixel, 1);

// -------- DIFFUSE TEXTURE BUFFER, pixel reg t0 ------------

	if(texture)
		deviceContext->PSSetShaderResources(0, 1, &texture);

// -------- NORMAL MAP BUFFER, pixel reg t1 ------------
	
	if(normalMap)
		deviceContext->PSSetShaderResources(1, 1, &normalMap);

// -------- SHADOW MAPS BUFFER, pixel reg t2-t9 ------------

	std::vector<ID3D11ShaderResourceView*> shadowMapsTextures;
	shadowMapsTextures.resize(NUMOFLIGHTS);
	if(shadowMaps)
		for (int i = 0; i < shadowMaps->size(); i++) 
			shadowMapsTextures[i] = (*shadowMaps)[i]->getDepthMapSRV();

	deviceContext->PSSetShaderResources(2, NUMOFLIGHTS, shadowMapsTextures.data());

// -------- SAMPLER BUFFERS, pixel reg s1-s2 ------------

	deviceContext->PSSetSamplers(0, 1, &_sampleState);
	deviceContext->PSSetSamplers(1, 1, &_sampleStateShadow);
}

void DefaultShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	//! Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	//! Setup the constant buffers.
	setupBuffer<MatrixBufferType>(renderer, &_matrixBuffer);
	setupBuffer<LightBufferType>(renderer, &_lightBuffer);
	setupBuffer<LightMatrixBufferType>(renderer, &_lightMatrixBuffer);
	setupBuffer<MaterialBufferType>(renderer, &_materialBuffer);
	setupBuffer<CameraBufferType>(renderer, &_cameraBuffer);

	//! Setup samplers
	D3D11_TEXTURE_ADDRESS_MODE m = D3D11_TEXTURE_ADDRESS_WRAP;
	setupSampler(renderer, _sampleState, m, m, m);

	m = D3D11_TEXTURE_ADDRESS_BORDER;
	setupSampler(renderer, _sampleStateShadow, m, m, m, 1.f,1.f,1.f,1.f);
}

//! etract light data into a buffer struct, later mapped to the actual light buffer
void DefaultShader::LightToShaderBuffer(Light& light, LightBufferType* buffer, LightType type, int ID)
{
	buffer->L_ambient[ID] = light.getAmbientColour();
	buffer->L_diffuse[ID] = light.getDiffuseColour();
	buffer->L_specular[ID] = light.getSpecularColour();
	buffer->L_DirType[ID].direction = light.getDirection();
	buffer->L_DirType[ID].type = (int)type;
	buffer->L_PosCut[ID].position = light.getPosition();
	buffer->L_PosCut[ID].cutoff = 40;
}

