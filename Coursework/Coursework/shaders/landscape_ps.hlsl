#include "shader_tools_ps.hlsli"
#include "Constants.hlsli"

// BUFFERS //

Texture2D layerTexture[3] : register(t10); //! t1-t9 are reserved for the default shader textures
Texture2D normalMaps[3] : register(t13);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos[NUM_OF_LIGHTS] : TEXCOORD3;
};

cbuffer LandscapeBuffer : register(b2)
{
    float4 ranges;
};

// FUNCTIONS //

float4 main(InputType input) : SV_TARGET
{
//----------------NORMAL AND TEXTURE -----------------
	
    //! constants for alpha and normal map handlig
    float3 normalVector = input.normal;
    float4 alpha;
    float4 normalMapValue = float4(0.f, 0.f, 0.f, 0.f);
    float2 finalUV = applyUVTransform(input.tex, uvOffset, uvScale);
    
    //! interpolate beteen layers if needed, based on the altitude
    //Layer 1
    if (input.worldPosition.y < ranges.x)
    {
        
        alpha = getScaledTextureColour(input.tex, uvOffset, uvScale, layerTexture[0]);
        normalVector = handleSpecificNormalMap(normalVector, finalUV, normalMaps[0]);
    }
    
    //Layer 1-2
    else if (input.worldPosition.y < ranges.y)
    {
        alpha = interpolateColourFromRange(
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 1.f) ? layerTexture[0].SampleLevel(diffuseSampler, finalUV, 0) : diffuse),
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 1.f) ? layerTexture[1].SampleLevel(diffuseSampler, finalUV, 0) : diffuse),
            ranges.x,
            ranges.y,
            input.worldPosition.y
        );
        normalMapValue = interpolateColourFromRange(
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 2.f) ? normalMaps[0].SampleLevel(diffuseSampler, finalUV, 0) : float4(0.5f, 0.5f, 0.5f, 0.f)),
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 2.f) ? normalMaps[1].SampleLevel(diffuseSampler, finalUV, 0) : float4(0.5f, 0.5f, 0.5f, 0.f)),
            ranges.x,
            ranges.y,
            input.worldPosition.y
        );
        
        normalVector.x += (normalMapValue.r - 0.5f);
        normalVector.y += (normalMapValue.g - 0.5f);
        normalVector.z += (normalMapValue.b - 0.5f);
    
        normalVector = normalize(normalVector);
    }
    
    //Layer 1
    else if (input.worldPosition.y < ranges.z)
    {
        alpha = getScaledTextureColour(input.tex, uvOffset, uvScale, layerTexture[1]);
        normalVector = handleSpecificNormalMap(normalVector, finalUV, normalMaps[1]);
    }
    
    //Layer 2-3
    else if (input.worldPosition.y < ranges.w)
    {
        alpha = interpolateColourFromRange(
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 1.f) ? layerTexture[1].SampleLevel(diffuseSampler, finalUV, 0) : diffuse),
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 1.f) ? layerTexture[2].SampleLevel(diffuseSampler, finalUV, 0) : diffuse),
            ranges.z,
            ranges.w,
            input.worldPosition.y
        );
        normalMapValue = interpolateColourFromRange(
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 2.f) ? normalMaps[1].SampleLevel(diffuseSampler, finalUV, 0) : float4(0.5f, 0.5f, 0.5f, 0.f)),
            saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 2.f) ? normalMaps[2].SampleLevel(diffuseSampler, finalUV, 0) : float4(0.5f, 0.5f, 0.5f, 0.f)),
            ranges.x,
            ranges.y,
            input.worldPosition.y
        );
        
        normalVector.x += (normalMapValue.r - 0.5f);
        normalVector.y += (normalMapValue.g - 0.5f);
        normalVector.z += (normalMapValue.b - 0.5f);
    
        normalVector = normalize(normalVector);
    }
    
    //Layer 3
    else
    {
        alpha = getScaledTextureColour(input.tex, uvOffset, uvScale, layerTexture[2]);
        normalVector = handleSpecificNormalMap(normalVector, finalUV, normalMaps[2]);
    }

    
//---------------- LIGHTS AND SHADOWS ---------
    
    Light_Colours lightData = handleLights(
        input.lightViewPos, 
        normalVector,
        input.worldPosition, 
        input.viewVector,
        input.tex);
    
//-------- final assembly of the colour ----------
    lightData.lightColour = finalizeLightColour(lightData.shadowPasses, lightData.ambient, lightData.lightColour);
    
    float4 finalShade = VALID_ADD(emissive);
    finalShade += ((lightData.lightColour * alpha) * (1 - emissive.w));

    return saturate(finalShade + saturate(lightData.specular));
    //return float4(normalVector.x / 2 + .5f, normalVector.y / 2 + .5f, normalVector.z / 2 + .5f, 1);
}