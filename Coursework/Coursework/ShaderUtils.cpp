#include "ShaderUtils.h"
#include "Object.h"
#include "PPBlurShader.h"

void setupSampler(
	ID3D11Device* renderer,
	ID3D11SamplerState* sampleState_,
	D3D11_TEXTURE_ADDRESS_MODE modeU,
	D3D11_TEXTURE_ADDRESS_MODE modeV,
	D3D11_TEXTURE_ADDRESS_MODE modeW,
	float borderColor_R,
	float borderColor_G,
	float borderColor_B,
	float borderColor_A,
	D3D11_FILTER filter,
	float MipLoadBias,
	int MaxAnisotropy,
	D3D11_COMPARISON_FUNC cmpFunc,
	float MinLOD,
	float MaxLOD)
{
	//! create sampker description
	D3D11_SAMPLER_DESC samplerDesc;

	//! setup sampler description
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = modeU;
	samplerDesc.AddressV = modeV;
	samplerDesc.AddressW = modeW;
	samplerDesc.MipLODBias = filter;
	samplerDesc.MaxAnisotropy = MaxAnisotropy;
	samplerDesc.ComparisonFunc = cmpFunc;
	samplerDesc.BorderColor[0] = borderColor_R;
	samplerDesc.BorderColor[1] = borderColor_G;
	samplerDesc.BorderColor[2] = borderColor_B;
	samplerDesc.BorderColor[3] = borderColor_A;
	samplerDesc.MinLOD = MinLOD;
	samplerDesc.MaxLOD = MaxLOD;

	//! assign/bind sampler description
	renderer->CreateSamplerState(&samplerDesc, &sampleState_);
}

void finalizeBuffer(ID3D11DeviceContext* context, ID3D11Buffer* buffer, PipelineStage stage, int bufferID)
{
	//! unmap and propagate the buffer to the correct shader stage and register
	context->Unmap(buffer, 0);
	switch (stage)
	{
	case Vertex:
		context->VSSetConstantBuffers(bufferID, 1, &buffer);
		break;
	case Pixel:
		context->PSSetConstantBuffers(bufferID, 1, &buffer);
		break;
	case Geo:
		context->GSSetConstantBuffers(bufferID, 1, &buffer);
		break;
	case Domain:
		context->DSSetConstantBuffers(bufferID, 1, &buffer);
		break;
	case Hull:
		context->HSSetConstantBuffers(bufferID, 1, &buffer);
		break;
	case Compute:
		context->CSSetConstantBuffers(bufferID, 1, &buffer);
	}
}

void ReleaseBuffer(ID3D11Buffer** buffer)
{
	//! check if deleteable and if so delete and cleanup the original pointer
	if (*buffer)
	{
		(*buffer)->Release();
		(*buffer) = 0;
	}
}

void ReleaseSampler(ID3D11SamplerState** sampler)
{
	//! check if deleteable and if so delete and cleanup the original pointer
	if (*sampler)
	{
		(*sampler)->Release();
		(*sampler) = 0;
	}
}

void BakeLightsMaps(D3D* renderer, std::vector<ShadowMap*>& maps, std::vector<Light*>& lights, std::vector<Object*>& sceneObjects, XMMATRIX& view, HWND win)
{
	//! Variables for defining shadow map
	const int _shadowmapWidth = 2048;
	const int _shadowmapHeight = 2048;

	//! iterate through all lights and build their respective light maps
	for (int i = 0; i < lights.size(); i++) {

		//! add the map if not represented yet
		if(i >= maps.size())
			maps.push_back(new ShadowMap(renderer->getDevice(), _shadowmapWidth, _shadowmapHeight));
		
		//! draw to a specified map
		maps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		//! get the world, view, and projection matrices from the camera and d3d objects.
		XMMATRIX lightViewMatrix = lights[i]->getViewMatrix();
		XMMATRIX lightProjectionMatrix = lights[i]->getOrthoMatrix();

		//! Render objects with low render
		for (auto it : sceneObjects)
		{
			//! diffuse w acts as a flag here to exclude stuff like water
			if (it->getMaterial()->diffuse.w < -1.f)
				continue;

			XMMATRIX worldMatrix = renderer->getWorldMatrix();
			it->render(renderer, lightViewMatrix, lightProjectionMatrix);
		}
		renderer->resetViewport();
	}

	renderer->setBackBufferRenderTarget();
}

void BlurTexture(D3D* renderer, ID3D11ShaderResourceView* toBlur, PPBlurShader* shader, XMFLOAT2 resolution, XMFLOAT2 blurDimensions, XMMATRIX& view, RenderTexture* result)
{
	//! create ortho mesh to render the texture onto
	auto* orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), resolution.x, resolution.y, 0, 0);
	auto world = renderer->getWorldMatrix();

	//! specify where to render
	result->setRenderTarget(renderer->getDeviceContext());
	result->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);
	auto ortho = result->getOrthoMatrix();

	//!disables z test
	renderer->setZBuffer(false);
	
	//! box blur kernel
	float cummulativeWeight = 9.0f;
	XMFLOAT3X3 kernel = XMFLOAT3X3(
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f);

	//! render procedure
	orthoMesh->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), world, view, ortho, toBlur, blurDimensions, resolution);
	shader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->resetViewport();
	
	//! cleanup from here
	renderer->setZBuffer(true);

	///! Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();

	delete orthoMesh;
}

XMFLOAT2 UVPanner(XMFLOAT2 currentOffsetUV, XMFLOAT2 speed)
{
	//! update the given uv offset by speed and time, inear function
	XMFLOAT2 newUV = XMFLOAT2(currentOffsetUV.x + speed.x, currentOffsetUV.y + speed.y);
	newUV = XMFLOAT2(
		newUV.x - (int)newUV.x < 0.f ? 1 - (newUV.x - (int)newUV.x) : newUV.x - (int)newUV.x,
		newUV.y - (int)newUV.y < 0.f ? 1 - (newUV.y - (int)newUV.y) : newUV.y - (int)newUV.y
	);
	return newUV;
}

//! EXTERNAL
//! https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.cpp
HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
{
	*ppBufOut = nullptr;
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = uElementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = uElementSize;
	if (pInitData)
	{
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
	}
	else
		return pDevice->CreateBuffer(&desc, nullptr, ppBufOut);
}

//! EXTERNAL
//! https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.cpp
HRESULT CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		desc.Buffer.NumElements = descBuf.ByteWidth / 4;
	}
	else
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
			desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return E_INVALIDARG;
		}

	return pDevice->CreateUnorderedAccessView(pBuffer, &desc, ppUAVOut);
}

//! EXTERNAL
//! https://github.com/walbourn/directx-sdk-samples/blob/master/BasicCompute11/BasicCompute11.cpp
ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer)
{
	ID3D11Buffer* debugbuf = nullptr;

	D3D11_BUFFER_DESC desc = {};
	pBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	if (SUCCEEDED(pDevice->CreateBuffer(&desc, nullptr, &debugbuf)))
	{
#if defined(_DEBUG) || defined(PROFILE)
		debugbuf->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Debug") - 1, "Debug");
#endif

		pd3dImmediateContext->CopyResource(debugbuf, pBuffer);
	}

	return debugbuf;
}