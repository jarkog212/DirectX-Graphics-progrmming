#pragma once

#ifndef _GPU_ORDER_SHADER_H_
#define _GPU_ORDER_SHADER_H_

#include "DXF.h"
#include "BaseMesh.h"
#define MAX_NUM_TO_SORT 5000       //! max limit of elements to sort per dispatc call should not be changed

using namespace std;
using namespace DirectX;

class GPUOrderShader :
    public BaseShader
{
	
	//! used for sorting, hacky replacement for global variables in compute shader
	//! used as a part of the UAV buffer
	struct VertexContainerTemplate
	{
		float ID;
		float distance;
		XMFLOAT2 p2;
	};

	//! reduced Vertex type as only position is needed
	struct VertexType 
	{
		XMFLOAT3 position;
		float p;
	};

public:

	//! full UAV buffer, one per thread, can be accessed by other threads though
	struct ComputeBufferType
	{
		VertexType points;
		VertexContainerTemplate unordered;
	};

	//! buffer for the array of points, has to be done in 4 parts due to the size limitations per buffer
	struct VertexListBufferType 
	{
		VertexType list[MAX_NUM_TO_SORT / 4];
	};

	//! specifies artributes for the caluclation and camera position since its constant
	struct ComputeConstantsBufferType 
	{
		float numOfPoints;
		XMFLOAT3 cameraPosition;
		float numOfPasses = 3;
		XMFLOAT3 p;
	};

	GPUOrderShader(ID3D11Device* device, HWND hwnd);
	~GPUOrderShader();

	//! this is only compute shader, no other shaders are needed
	void setShaderParameters(ID3D11DeviceContext* deviceContext) {};

	//! not a compute override, its own function, does not inherit form DefaultShader
	void Compute(D3D* renderer, std::vector<BaseMesh::VertexType>* vertices, XMFLOAT3 cameraPos);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) {};

private:
	ID3D11Buffer* _computeConstantBuffer = NULL;
	//! 4 buffers due to memory limitations per buffer 
	ID3D11Buffer* _vertexListBuffer[4] = {};
	ID3D11Buffer* _computeBuffer = NULL;
};

#endif