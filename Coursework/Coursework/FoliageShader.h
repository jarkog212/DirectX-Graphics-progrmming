#pragma once

#ifndef _FOLIAGE_SHADER_H_
#define _FOLIAGE_SHADER_H_

#include "DefaultShader.h"
#include "BetterPointMesh.h"

#define TEX_HEIGHT 1024   //!resolution of the brush map
#define TEX_WIDTH 1024    //!...

class FoliageShader :
    public DefaultShader
{
    //! buffer for cumpute constants, same for all threads
    struct ConstantComputeBufferType
    {
        float filingPercentage_GPU = 0.5f;
        XMFLOAT3 landscapeScalng = XMFLOAT3(100.f, 1.f, 100.f);
    };

    //! additional buffer for random size of each instance of foliage
    struct TransformBufferType 
    {
        XMFLOAT3 scalingRangeBottom; //min in all axes
        float padding;
        XMFLOAT3 scalingRangeTop;    //max in all axes
        float padding2;
    };

public:

    //! buffer for vertex generation compute buffer
    struct VertexComputeBufferType
    {
        XMFLOAT3 position;
        float p;
        XMFLOAT2 texture;
        XMFLOAT2 p2;
        XMFLOAT3 normal;
        float p3;
    };

    //! ddefined set of params for addional params
    struct FoliageParams
    {
        XMFLOAT3 scalingRangeBottom;
        XMFLOAT3 scalingRangeTop;
    };

    FoliageShader(ID3D11Device* device, HWND hwnd);
    ~FoliageShader();

    //! set of compute params for vertex generation
    struct ComputeParams
    {
        ID3D11ShaderResourceView* heightMap;
        ID3D11ShaderResourceView* brushMap;
        ConstantComputeBufferType additionalParams;
    };

    void Compute(D3D* renderer, void* computeParams) override;

private:
    void additionalParameters(ID3D11DeviceContext* device, void* params) override;

    ID3D11Buffer* _constantComputeBuffer = NULL;
    ID3D11Buffer* _transformBuffer = NULL;

public:
    //! public to allow for direct access when reading the results of the compute
    ID3D11Buffer* _vertexComputeBuffer = NULL;
};

#endif