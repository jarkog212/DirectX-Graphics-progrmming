#include "shader_tools_vs.hlsli"
#define MAX_ALTITUDE 50.f

// BUFFERS //

Texture2D heightMap : register(t0);
SamplerState heightSampler : register(s0);

struct InputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos[NUM_OF_LIGHTS] : TEXCOORD3;
};

// FUNCTIONS //

float3 calculateNormalFromHeightMap(Texture2D heighMap, float2 uv, float step)
{
    const float deltaY1 = heightMap.SampleLevel(heightSampler, uv + float2(step, 0.f), 0) - heightMap.SampleLevel(heightSampler, uv + float2(-step, 0.f), 0);
    const float deltaY2 = heightMap.SampleLevel(heightSampler, uv + float2(0.f, step), 0) - heightMap.SampleLevel(heightSampler, uv + float2(0.f, -step), 0);
    return calculateNormalFromPlane(step, deltaY1, deltaY2);
}

OutputType main(InputType input)
{
    OutputType output;

    const float4 heightVal = heightMap.SampleLevel(heightSampler, input.tex, 0);
    float4 relativePos = input.position;
    relativePos.y += heightVal.r * MAX_ALTITUDE;

	//! Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = calculateScreenPosition(relativePos);
    output.worldPosition = calculateWorldPosition(relativePos);
    
	//! Calculate the position of the vertice as viewed by the light source.
    for (int i = 0; i < NUM_OF_LIGHTS; i++)
        output.lightViewPos[i] = calculateLightViewPosition(relativePos, i);

    //! propagate normals
    output.tex = input.tex;
    
    //! Recalculate normals for vertices
    //! Since uv are 0-1 and height map is 0-1, we can relate height differences with positional differences 1:1
    const float3 normal = calculateNormalFromHeightMap(heightMap, input.tex, 0.008);
    output.normal = calculateWorldNormal(normal);

    //! View vector per vertex
    output.viewVector = calculateCameraView(output.worldPosition);
    
    return output;
} 