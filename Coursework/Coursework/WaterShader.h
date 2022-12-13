#pragma once

#ifndef _WATER_SHADER_H_
#define _WATER_SHADER_H_

#include "DefaultShader.h"

class WaterShader :
    public DefaultShader
{
public:
    //! Buffer for waves in pixel shader, second layer offset and scale, public for UI params ease of access
    struct WaterPixelBufferType 
    {
        XMFLOAT2 uvOffset2 = { 0, 0 };
        XMFLOAT2 uvScaling2 = { 1, 1 }; //altitude, frequency
        XMFLOAT3 landscapeOriginPosition = { 0,0,0 };
        float p;
    };

    //! buffer for vertex stage, modifies the waves, public for UI params ease of access 
    struct WaterVertexBufferType
    {
        float time = 0.f;
        float waveAltitude = 1.f;
        float waveFrequency = 1.f;
        float p2;
    };

    //! additional params for water shader, inlcudes the second layer under layer of textures
    struct WaterParams 
    {
        WaterPixelBufferType pixelBuffer;
        WaterVertexBufferType vertexBuffer;
        ID3D11ShaderResourceView* bottomLayer;
        ID3D11ShaderResourceView* heightMap;
    };

    WaterShader(ID3D11Device* device, HWND hwnd);
    ~WaterShader();

private:
    void additionalParameters(ID3D11DeviceContext* device, void* params) override;

    ID3D11Buffer* _waterBuffer = NULL;
    ID3D11Buffer* _waveBuffer = NULL;
public:

};

#endif