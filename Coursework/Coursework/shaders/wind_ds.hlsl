// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
#include "shader_tools_vs.hlsli"
// BUFFERS //

cbuffer WindBuffer : register(b3)
{
    float intensity;
    float3 p;
    float2 uvScale;
    float2 uvOffset;
};

Texture2D windMap : register(t0);
Texture2D windBrush : register(t1);

SamplerState diffuseSampler : register(s0);

// STRUCTS //

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
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

// FUNKCTIONS //

[domain("tri")]
OutputType main(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 3> patch)
{
    OutputType output;
    
    //! calculate the UVs, flip Y coords
    float2 texUVs = uvwCoord.x * patch[0].tex + uvwCoord.y * patch[1].tex + uvwCoord.z * patch[2].tex;
    output.tex = flipUVsVertical(texUVs);
    
    //! Determine the position of the new vertex.
	//! Invert the y and Z components of uvwCoord as these coords are generated in UV space and therefore y is positive downward.
	//! Alternatively you can set the output topology of the hull shader to cw instead of ccw (or vice versa).    
    float4 vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;
    
    //! wind offset calculation, sampled form the map
    float2 windUV = applyUVTransform(output.tex, uvOffset, uvScale);
    float4 positionOffset = windMap.SampleLevel(diffuseSampler, windUV, 0);
    float4 brush = windBrush.SampleLevel(diffuseSampler, output.tex, 0);
    positionOffset -= float4(.5f, .5f, .5f, 0.f);
    positionOffset *= brush;
    vertexPosition += positionOffset * intensity;
    
    //! Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = calculateScreenPosition(vertexPosition);
    output.worldPosition = calculateWorldPosition(vertexPosition);
    
	//! Calculate the position of the vertex as viewed by the light source.
    for (int i = 0; i < NUM_OF_LIGHTS; i++)
        output.lightViewPos[i] = calculateLightViewPosition(vertexPosition, i);
    
    //! calculate normals in the world space
    float3 normals = uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal + uvwCoord.z * patch[2].normal;
    output.normal = calculateWorldNormal(normals);

    //! camera view vector calculation
    output.viewVector = calculateCameraView(output.worldPosition);
    
    return output;
}

