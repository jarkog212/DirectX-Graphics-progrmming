#include "shader_tools_ps.hlsli"

// BUFFERS //

Texture2D underTexture : register(t10);
Texture2D heightMap : register(t11); 
//! Landscape base UVs need to match the water plane 

cbuffer WaterPixelBuffer : register(b2)
{
    float2 uvOffset2;
    float2 uvScaling2;
    float3 landscapeOriginPosition;
    float p1;
};

struct InputType
{
    float4 position : S7V_POSITION;
    
    
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos[NUM_OF_LIGHTS] : TEXCOORD3;
};

// FUNCTIONS //

float4 main(InputType input) : SV_TARGET
{
//----------------NORMAL MAP-----------------
	
    float3 normalVector = handleNormalMap(input.normal, input.tex);
    
    //! body of the handle normal map function, separate to access shader buffers
    float2 uv = input.tex;
    uv = applyUVTransform(uv, uvOffset2, uvScaling2);
    
    normalVector = handleSpecificNormalMap(normalVector, uv, normalMapTexture);
    
//----------------LIGHTS AND SHADOWS---------
	
    Light_Colours lightData = handleLights(
        input.lightViewPos,
        normalVector,
        input.worldPosition,
        input.viewVector,
        input.tex);
 
    //handle metallic, requires reflecions, stretch goal
	//handle proper rougness, stretch goal - hard as that will require reflections, reflect at certain angle opposite to camera
    
    //! extracted assemble funciton, done to allow for multiple textures
    float4 alpha = getAlphaColour(input.tex);
    alpha += getScaledTextureColour(input.tex, uvOffset2, uvScaling2, underTexture);
    
    lightData.lightColour = finalizeLightColour(lightData.shadowPasses, lightData.ambient, lightData.lightColour);
	
    float4 finalShade = VALID_ADD(emissive);
    finalShade += ((lightData.lightColour * alpha.w * alpha) * (1 - emissive.w));

    float4 _out = saturate(finalShade + saturate(lightData.specular));
    _out.w = saturate(alpha.w + length(lightData.specular.xyz));
    
    //water foam around the shore/edge
    float heightVal = landscapeOriginPosition.y + heightMap.Sample(diffuseSampler, input.tex).r * MAX_ALTITUDE;
    heightVal = abs(heightVal - input.worldPosition.y);
    _out += interpolateColourFromRange(float4(1.f, 1.f, 1.f, 0.f), float4(0.01f, 0.01f, 0.01f, 0.01f), 0.f, 1.5f, heightVal);
    
    return _out;
    //return float4(normalVector.x / 2 + .5f, normalVector.y / 2 + .5f, normalVector.z / 2 + .5f, 1);

}
