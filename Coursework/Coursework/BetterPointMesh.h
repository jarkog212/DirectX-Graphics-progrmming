#pragma once
#include "../include/PointMesh.h"
#include <vector>
class BetterPointMesh :
    public PointMesh
{

public:
	BetterPointMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext) : PointMesh(device, deviceContext) { vertexCount_ = 0; indexCount_ = 0; };
	~BetterPointMesh() { PointMesh::~PointMesh();};
	
	//!adds the vertex to the list and updates the buffers. 
	void addVertex(ID3D11Device* device, VertexType toAdd, XMFLOAT3 positionOffset = {0,0,0});
	
	std::vector<VertexType>* getVerticesVector() { return &vertices_; }
	void initBuffers(ID3D11Device* device);

protected:
	std::vector<VertexType> vertices_;

};

