#pragma once
#ifndef _LANDSCAPE_SHADER_H_
#define _LANDSCAPE_SHADER_H_
#include "DefaultShader.h"

class LandscapeShader :
    public DefaultShader
{
    //! ranges buffer specifies the altitudes at which the layers blend or change
    struct LandscapeBufferType 
    {
        XMFLOAT4 ranges;
    };

public:
    //! additional shader params for landscape shaders
    struct LandscapeParameters
    {
        ID3D11ShaderResourceView* heightMap = NULL;
        ID3D11ShaderResourceView* bottomL = NULL;
        ID3D11ShaderResourceView* middleL = NULL;
        ID3D11ShaderResourceView* topL = NULL;
        ID3D11ShaderResourceView* bottomL_N = NULL;
        ID3D11ShaderResourceView* middleL_N = NULL;
        ID3D11ShaderResourceView* topL_N = NULL;
        std::pair<float, float> bot_mid_range = { 0.f,0.f };
        std::pair<float, float> mid_top_range = { 0.f,0.f };
        float maxAltitude; //unused
    };

    LandscapeShader(ID3D11Device* device, HWND hwnd);
    ~LandscapeShader();
private:
    void additionalParameters(ID3D11DeviceContext* device,void* params) override;
    
    ID3D11Buffer* _landscapeBuffer = NULL;
};

#endif