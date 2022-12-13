#include "shader_tools_vs.hlsli"

// BUFFERS //

cbuffer WaterVertexBuffer : register(b3)
{
    float time;
    float waveAltitude;
    float waveFrequency; //altitude, frequency
    float p2;
};
    
struct InputType
{
    float4 position : POSITION;
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

float getAltitudeAt(float3 position)
{
    float altitudeOffset = waveAltitude * (sin(position.x * waveFrequency + time) + cos(position.z * waveFrequency + time));
    return position.y + altitudeOffset;
}

float3 calculateNormal(float3 position, float step)
{
    const float line1Y = getAltitudeAt(float3(position.x + step / 2.f, position.y, position.z)) - getAltitudeAt(float3(position.x - step / 2.f, position.y, position.z));
    const float line2Y = getAltitudeAt(float3(position.x, position.y, position.z + step / 2.f)) - getAltitudeAt(float3(position.x, position.y, position.z - step / 2.f));
    return calculateNormalFromPlane(step, line1Y, line2Y);
}

OutputType main(InputType input)
{
    OutputType output;
    
    //! waves
    float4 position = input.position;
    position.y = getAltitudeAt(position.xyz);
    
	//! Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = calculateScreenPosition(position);
    output.worldPosition = calculateWorldPosition(position);
    
	//! Calculate the position of the vertex as viewed by the light source.
    for (int i = 0; i < NUM_OF_LIGHTS; i++)
        output.lightViewPos[i] = calculateLightViewPosition(position, i);

    //! propagate UVs further
    output.tex = input.tex;
    
    //! calculate normal using basic calculus
    const float3 normal = calculateNormal(input.position.xyz,0.0002);
    
    //! calculate normals in the world space
    output.normal = calculateWorldNormal(normal);

    //! camera view vector calculation
    output.viewVector = calculateCameraView(output.worldPosition);
    
    return output;
}