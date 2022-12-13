#pragma once

#ifndef _WIND_SHADER_H_
#define _WIND_SHADER_H_

#include "DefaultShader.h"

class WindShader :
    public DefaultShader
{
public:

    //! buffer for camera data and max tess factor to be used by hull buffer
    struct HullBufferType 
    {
        XMFLOAT3 cameraPos;
        float maxTesselationFactor = 3;
    };

    //! buffer for domain shader, scales nad moves the wind displacement texture
    struct WindBufferType 
    {
        float intensity = 1;
        XMFLOAT3 p;
        XMFLOAT2 uvScale = { 1, 1 };
        XMFLOAT2 uvOffset = { 0,0 };
    };

    //! additional params for the wind shader
    struct WindAddititonalParams 
    {
        HullBufferType hullBuffer;
        WindBufferType windBuffer;
        ID3D11ShaderResourceView* windTexture;
        ID3D11ShaderResourceView* windBrushTexture;
    };

    WindShader(ID3D11Device* device, HWND hwnd);
    ~WindShader();

private:
    void additionalParameters(ID3D11DeviceContext* device, void* params) override;

    ID3D11Buffer* _hullBuffer = NULL;
    ID3D11Buffer* _windBuffer = NULL;
};

#endif