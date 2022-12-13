#include "BetterPointMesh.h"
#include "ShaderUtils.h"
#include <vector>

//! add a vertex to the point list, used for geometry shader
void BetterPointMesh::addVertex(ID3D11Device* device, VertexType toAdd, XMFLOAT3 positionOffset)
{
	toAdd.position.x += positionOffset.x;
	toAdd.position.y += positionOffset.y;
	toAdd.position.z += positionOffset.z;
	vertices_.push_back(toAdd);
	vertexCount_++;
	indexCount_ = vertexCount_;

	//! recalculates the indices and vertex buffers.
	initBuffers(device);
}

//! recalculates the incdeces and vertices, adds them to the buffer
void BetterPointMesh::initBuffers(ID3D11Device* device)
{
	//! remove old buffers
	ReleaseBuffer(&vertexBuffer);
	ReleaseBuffer(&indexBuffer);

	//! copy the list of the orihinal vertices, used as a way to preserve the points betwen calls
	std::vector<VertexType> vertices;
	vertices.resize(vertexCount_);
	memcpy(vertices.data(), vertices_.data(), sizeof(VertexType) * vertexCount_);

	//! define the arrays and descriptors
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	indices = new unsigned long[indexCount_];

	//! fill up indices array, default order, presumes the vertices are already ordered desirably
	for (int i = 0; i < vertexCount_; i++) 
		indices[i] = i;  

	//! Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount_;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	//! Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	//! Now create the vertex buffer. Seems to mess around the input array
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	//! Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount_;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	//! Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	//! Create the index buffer. Seems to mess arounf with the input array
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	//! cleanup
	vertices.clear();
	delete[] indices;
	indices = 0;
}
