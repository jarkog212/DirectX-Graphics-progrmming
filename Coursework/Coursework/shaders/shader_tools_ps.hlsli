#include "Constants.hlsli"

#define VALID_ADD(colour) isnan(colour.x) ? float4(0.f,0.f,0.f,0.f) : (saturate(colour) * colour.w)

// STRUCTS //

//! default structs - not buffers
struct Position_Cutoff
{
    float3 position;
    float cutoff;
};

struct Direction_Type
{
    float3 direction;
    float type;
};

struct Light_Colours
{
    float4 lightColour;
    float4 specular;
    float4 ambient;
    float shadowPasses;
};

// BUFFERS //

//! default buffers used for most derived shaders
Texture2D shaderTexture : register(t0);
Texture2D normalMapTexture : register(t1);
Texture2D depthMapTexture[NUM_OF_LIGHTS] : register(t2); //reserve the t2 - t9 for light maps

SamplerState diffuseSampler : register(s0);
SamplerState shadowSampler : register(s1);

cbuffer MaterialBuffer : register(b0)
{
    float4 diffuse;
    float4 emissive;
    float roughness;
    float metallic;
    float2 uvScale;
    float2 uvOffset;
    float shadingType;
    float p;  
};

cbuffer LightBuffer : register(b1)
{
    float4 L_diffuse[NUM_OF_LIGHTS];
    float4 L_ambient[NUM_OF_LIGHTS];
    Position_Cutoff L_PosCut[NUM_OF_LIGHTS];
    Direction_Type L_DirType[NUM_OF_LIGHTS];
    float4 L_specular[NUM_OF_LIGHTS];
};

// FUNCTIONS //

//! FLIP UV VERTICAL //
//! Flips the y uv corrdinate
float2 flipUVsVertical(float2 uv)
{
    uv.y = 1 - uv.y;
    return uv;
};

//! APPLY UV TRANSFORM --------------------------------------------------------------------------------------------
//! applies arguments to UVs so that the final range is appropriately in range
float2 applyUVTransform(float2 uv, float2 offset, float2 scale)
{
    uv += offset;
    uv *= scale;
    uv = float2(frac(uv.x) , frac(uv.y));
    return uv;
}

//! GET SCALED TEXTURE COLOUR --------------------------------------------------------------------------------------------
//! samples the given texture, scales the uvs according to arguments
float4 getScaledTextureColour(float2 uv, float2 offset, float2 scale, Texture2D textureToSample)
{
    uv = applyUVTransform(uv, offset, scale);
    return saturate(textureToSample.SampleLevel(diffuseSampler, uv, 0));
}

//! GET ALPHA COLOUR --------------------------------------------------------------------------------------------
//! samples the texture if avilable, or returns diffuse colour
float4 getAlphaColour(float2 uv)
{
    return saturate(ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 1.f) ? getScaledTextureColour(uv, uvOffset,uvScale,shaderTexture) : diffuse);
}

//! CALCULATE PURE LIGHTING --------------------------------------------------------------------------------------------
//! calculates the light colour based on the type of the light
float4 calculatePureLighting(int lightID, float3 normal, float3 pixelWorldPosition)
{
    float intensity;
    float3 lightVector;
    float type = L_DirType[lightID].type;

    //Ambient only
    if (ENUM_IF(type, 3.f))
        return float4(0, 0, 0, 0);
    
    //Directional light
    if (ENUM_IF(type, 2.f))
    {
        intensity = saturate(dot(normal, normalize(-L_DirType[lightID].direction)));
        return saturate(L_diffuse[lightID] * intensity);
    }
	
    //Spotlight
    if (ENUM_IF(type, 1.f))
    {
        lightVector = normalize(pixelWorldPosition - L_PosCut[lightID].position);
        float3 unitLightDir = normalize(L_DirType[lightID].direction);

        float theta = dot(unitLightDir, lightVector);
        float maxTheta = cos((L_PosCut[lightID].cutoff / 2.0f) * (3.14f / 180.0f));

        intensity = saturate((theta - maxTheta) / (1.0f - maxTheta));
        return saturate(L_diffuse[lightID] * intensity);
    }
    
    //Point
    if (ENUM_IF(type, 0.f))
    {
        lightVector = L_PosCut[0].position - pixelWorldPosition;
        intensity = saturate(dot(normalize(lightVector), normal));

        const float constFactor = 0;
        const float linearFactor = 0.005;
        const float quadraticFactor = 0;

        float distance = length(lightVector);
        float attenuation = 1 / (constFactor + linearFactor * distance + quadraticFactor * pow(distance, 2));
	
        return saturate(L_diffuse[lightID] * intensity * attenuation);
    }
	
    //Error
    return float4(0, 0, 0, 0);
}

//! CALCULATE SPECULAR LIGHT --------------------------------------------------------------------------------------------
//! determines the colour, intesity and visibility of a specular spot 
float4 calculateSpecularLighting(int lightID, float3 worldPosition, float3 viewVector, float3 normal, float shadowPasses, float2 uv)
{
    float3 lightDirection = normalize(L_PosCut[lightID].position - worldPosition);
    
    float3 halfVector = normalize(lightDirection + viewVector);
    float specularIntensity = pow(max(dot(normal, halfVector), 0.0), saturate(1 - clamp(roughness, 0, 0.99)) * 128);
    float4 specular = L_specular[lightID] * specularIntensity * (1 - clamp(roughness, 0, 1));
    specular *= (shadowPasses / MAX_SHADOW_PASSES);
    return getAlphaColour(uv).w > 0.f ? specular : float4(0, 0, 0, 0);
}

//! HAS DEPTH DATA *code by the lecturers* --------------------------------------------------------------------------------------------
//! validates the uvs before sampling the shadow maps
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
        return false;

    return true;
}

//! IS IN SHADOW *code by the lecturers with modifications* --------------------------------------------------------------------------------------------
//! determines from shadow map whether the current pixel should be lit
int isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    const float stepCoeff = 0.0002;
    const int row = (int)sqrt(MAX_SHADOW_PASSES);
    const float2 tempUV = uv - (float2(-row/2.f, -row/2.f) * stepCoeff);
    int passes = 0;
	
    //! Sample the shadow map (get depth of geometry
    for (int i = 0; i < MAX_SHADOW_PASSES; i++)
    {
        float depthValue = sMap.Sample(shadowSampler, tempUV + (float2(i % row, i / row) * stepCoeff)).r;
	//! Calculate the depth from the light.
        float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
        lightDepthValue -= bias;

	//! Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
        if (lightDepthValue < depthValue)
            passes++;
    }
	
    return passes;
}

//! GET PROJECTIVE COORDS *code by the lecturers* --------------------------------------------------------------------------------------------
//! used for dhadow maps, returns texture coordinates projected based on the view position
float2 getProjectiveCoords(float4 lightViewPosition)
{
    //! Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

//! FINALIZE LIGHT COLOUR --------------------------------------------------------------------------------------------
//! determines the light intensity based on the shadowPasses, adds ambient as base
float4 finalizeLightColour(float shadowPasses, float4 ambient, float4 light)
{
    light *= (shadowPasses / MAX_SHADOW_PASSES);
    light += VALID_ADD(ambient);
    return saturate(light);
}

//! ASSEMBLE FINAL SHADE --------------------------------------------------------------------------------------------
//! uses calculations to determine the final result of the pixel shader
float4 assembleFinalShade(float2 uv, float shadowPasses, float4 light, float4 ambient, float4 specular)
{
    float4 alpha = getAlphaColour(uv);
    light = finalizeLightColour(shadowPasses, ambient, light);
	
    float4 finalShade = VALID_ADD(emissive);
    finalShade += ((light * alpha.w * alpha) * (1 - emissive.w));

    float4 _out = saturate(finalShade + saturate(specular));
    _out.w = saturate(alpha.w + length(specular.xyz));
    return _out;
}
//! HANDLE SPECIFIC NORMAL MAP --------------------------------------------------------------------------------------------
//! offsets the normal vector based on the base slot normal map texture, non scaled 
float3 handleSpecificNormalMap(float3 normal, float2 uv, Texture2D normalMap)
{
    float4 normalMapValue = ENUM_IF(shadingType, 0.f) || ENUM_IF(shadingType, 2.f) ? normalMap.SampleLevel(diffuseSampler, uv, 0) : float4(0.5f, 0.5f, 0.5f, 0.f);
    float3 normalVector = normal;
    
    normalVector.x += (normalMapValue.r - 0.5f);
    normalVector.y += (normalMapValue.g - 0.5f);
    normalVector.z += (normalMapValue.b - 0.5f);
    
    return normalize(normalVector);
}

//! HANDLE NORMAL MAP --------------------------------------------------------------------------------------------
//! offsets the normal vector based on the base slot normal map texture, uses the shader default normal map 
float3 handleNormalMap(float3 normal, float2 uv)
{
    uv = applyUVTransform(uv, uvOffset, uvScale);
    return handleSpecificNormalMap(normal, uv, normalMapTexture);
}

//! INTERPOLATE COLOUR FROM RANGE --------------------------------------------------------------------------------------------
//! interpolates between colours based on a given range and a value within it
float4 interpolateColourFromRange(float4 colour_0, float4 colour_1, float range_0, float range_1, float val )
{
    float factor = (val - range_0) / (range_1 - range_0);
    return lerp(colour_0, colour_1, saturate(factor));
}

//! HADNLE LIGHTS --------------------------------------------------------------------------------------------
//! uses parameters and shadow maps to see whehter and at what intensity to calculate light
Light_Colours handleLights(
    float4 lightViewPos[NUM_OF_LIGHTS],
    float3 normal,
    float3 worldPosition,
    float3 viewVector,
    float2 tex,
    float shadowMapBias = 0.005)
{
    Light_Colours _out ={
        float4(0.f, 0.f, 0.f, 1.f),
        float4(0.f, 0.f, 0.f, 1.f),
        float4(0.f, 0.f, 0.f, 1.f),
        0
    };

    bool wasNotInShadow = true;

    for (int i = 0; i < NUM_OF_LIGHTS; i++)
    {
        if (isnan(L_diffuse[i].x))
            break;
		
        float2 pTexCoord = getProjectiveCoords(lightViewPos[i]);

		//! add ambient, regardless of whether in shadow
        _out.ambient += L_ambient[i]; //probably should not just add, needs testing

        _out.shadowPasses = isInShadow(depthMapTexture[i], pTexCoord, lightViewPos[i], shadowMapBias);
		
        //! Shadow test, applies light only if not in shadow
        if (wasNotInShadow && hasDepthData(pTexCoord) && _out.shadowPasses > 0)
        {
            _out.lightColour += calculatePureLighting(i, normal, worldPosition);
            _out.specular += calculateSpecularLighting(i, worldPosition ,viewVector, normal, _out.shadowPasses, tex);
        }
        else if (hasDepthData(pTexCoord));
            wasNotInShadow = false;
    }
    
    return _out;
}