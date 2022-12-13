#include "GPUOrderShader.h"
#include "ShaderUtils.h"

GPUOrderShader::GPUOrderShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	//! uses defalt pixel shader, the other stages are ccustomized
	//! frankly the pixel and vertex buffers are not needed at all, but have to be defined
	initShader(L"depth_vs.cso", L"depth_ps.cso");
	loadComputeShader(L"gpu_order_cs.cso");

	//! create buffers
	CreateStructuredBuffer(device, sizeof(ComputeBufferType), MAX_NUM_TO_SORT, nullptr, &_computeBuffer);

	//! buffers are separated due to memory restrictions
	for (int i = 0; i < 4; i++)
		setupBuffer<VertexListBufferType>(renderer, &_vertexListBuffer[i]);

	setupBuffer<ComputeConstantsBufferType>(renderer, &_computeConstantBuffer);
}

GPUOrderShader::~GPUOrderShader()
{
	//! cleanup new buffers
	ReleaseBuffer(&_computeBuffer);
	ReleaseBuffer(&_computeConstantBuffer);

	for (int i = 0; i < 4; i++)
		ReleaseBuffer(&_vertexListBuffer[i]);

	//! cleanup inherited objects
	BaseShader::~BaseShader();
}

void GPUOrderShader::Compute(D3D* renderer, std::vector<BaseMesh::VertexType>* vertices, XMFLOAT3 cameraPos)
{
	// -------- COMPUTE CONSTANTS BUFFER, compute reg b0 ------------

	auto* constantPtr = MapBufferToPointer<ComputeConstantsBufferType>(renderer->getDeviceContext(), _computeConstantBuffer);
	constantPtr->cameraPosition = cameraPos;
	constantPtr->numOfPoints = vertices->size();
	finalizeBuffer(renderer->getDeviceContext(), _computeConstantBuffer, PipelineStage::Compute, 0);

	// -------- VERTEX LIST BUFFERS, compute reg b1-b4 ------------

	int count = vertices->size();
	for (int i = 0; i < 4 && count > 0; i++)
	{
		auto* dataPtr = MapBufferToPointer<VertexListBufferType>(renderer->getDeviceContext(), _vertexListBuffer[i]);
		for (int j = 0; j < min(MAX_NUM_TO_SORT / 4, count); j++)
			dataPtr->list[j].position = (*vertices)[j + (MAX_NUM_TO_SORT / 4) * i].position;
		finalizeBuffer(renderer->getDeviceContext(), _vertexListBuffer[i], PipelineStage::Compute, i + 1);            //+1 offset due to the computeConstantBuffer
		count -= MAX_NUM_TO_SORT / 4;
	}
	
	// -------- COMPUTE BUFFER, reg u0 ------------
	//! UAV only buffer, uses external code 
	ID3D11UnorderedAccessView* UAV;
	CreateBufferUAV(renderer->getDevice(), _computeBuffer, &UAV);
	renderer->getDeviceContext()->CSSetUnorderedAccessViews(0, 1, &UAV, nullptr);

	//! dispatch call, create 5 groups of 1000 threads each
	//! 5 groups each has 1000 threads = 5000 threads, a max of points to sort
	compute(renderer->getDeviceContext(), 5, 1, 1);

	//! copy resulting data from dispatch, as to not break the original buffer
	ID3D11Buffer* computeResult = CreateAndCopyToDebugBuf(renderer->getDevice(), renderer->getDeviceContext(), _computeBuffer);

	//! store back the sorted array and cleanup
	auto* resultPtr = MapBufferToPointer<ComputeBufferType[MAX_NUM_TO_SORT]>(renderer->getDeviceContext(), computeResult, (D3D11_MAP)D3D11_MAP_READ);
	for (int i = 0; i < min(MAX_NUM_TO_SORT,  vertices->size()); i++)
		(*vertices)[i].position = (*resultPtr)[i].points.position;
	renderer->getDeviceContext()->Unmap(computeResult, 0);
	ReleaseBuffer(&computeResult);
}
