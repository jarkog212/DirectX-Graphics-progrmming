#include "Constants.hlsli"
#include "external.hlsli"

#define HEIGHT 1024
#define WIDTH 1024

// BUFFER DEFINITIONS //

struct VertexBufferType
{
    float3 position;
    float p;
    float2 tex;
    float2 p2;
    float3 normal;
    float p3;
};

// BUFFERS //

Texture2D heightMap : register(t0);
Texture2D brushMap : register(t1);
RWStructuredBuffer<VertexBufferType> newVerticesBuffer : register(u0);
SamplerState diffuseSampler : register(s0);

cbuffer ConstantComputeBuffer : register(b0)
{
    float fillingPercentage;
    float3 landscapeScaling;
};

// GROUP CONSTANTS //

groupshared const float k_Invalid_Float = -15000.f;
groupshared const float texelWidth = 1.f / WIDTH;
groupshared const float texelHeight = 1.f / HEIGHT;

// FUNCTIONS //

[numthreads(1, 1024, 1)]
void main(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID )
{ 
    //! determine the correct UAV 
    //! uses y to better in dispatch as this better aps to the texture
    uint index = (groupID.x * WIDTH + groupThreadID.y);
    
    //! translate the thred ID to uvs
    //! possible as each thread corresponds to 1 pixel
    float2 uv = float2(groupThreadID.y * texelWidth, groupID.x * texelHeight);
    
    //! seed the random using position
    int rngSeed = groupID.x + dispatchThreadID.y * WIDTH;
    float random = nextFloat(rngSeed);
    
    //! only "spawn" a tree if lucky and brush map allows it, otherwise make it invalid
    if (random < fillingPercentage && 
        int(brushMap.SampleLevel(diffuseSampler, uv, 0).r))
    {
        float altitude = heightMap.SampleLevel(diffuseSampler, uv, 0).r * MAX_ALTITUDE;
        newVerticesBuffer[index].position = float3(uv.x * landscapeScaling.x, altitude, uv.y * landscapeScaling.z);
    }
    else
        newVerticesBuffer[index].position = float3(0.f, k_Invalid_Float, 0.f);
};