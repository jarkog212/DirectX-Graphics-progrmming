#include "Constants.hlsli"

// BUFFERS //

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBufferType : register(b1)
{
    float3 cameraPosition;
    float padding;
};

cbuffer LightMatrixBuffer : register(b2)
{
    matrix L_lightView[NUM_OF_LIGHTS];
    matrix L_lightProjection[NUM_OF_LIGHTS];
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
    uv = float2(frac(uv.x), frac(uv.y));
    return uv;
}

//! CALCULATE WORLD POSITION --------------------------------------------------------------------------------------------
//! applies the world matrix to a given vertex position
float4 calculateWorldPosition(float4 relativePos)
{
    return mul(relativePos, worldMatrix);
}

//! CALCULATE SCREEN POSITION --------------------------------------------------------------------------------------------
//! gets the exact screen plane position of a vertex
float4 calculateScreenPosition(float4 relativePos)
{
    float4 outPos = calculateWorldPosition(relativePos);
    outPos = mul(outPos, viewMatrix);
    return mul(outPos, projectionMatrix);
}

//! CALCULATE WORLD NORMAL --------------------------------------------------------------------------------------------
//! calculates world-relative normal vector
float3 calculateWorldNormal(float3 relativeNormal)
{
    return normalize(mul(relativeNormal, (float3x3)worldMatrix));
}

//! CALCULATE CAMERA VIEW --------------------------------------------------------------------------------------------
//! calculates vector for camera view
float3 calculateCameraView(float3 worldPos)
{
    return normalize(cameraPosition.xyz - worldPos.xyz);
}

//! CALCULATE LIGHT VIEW POSITION --------------------------------------------------------------------------------------------
//! calculates vector for light view
float4 calculateLightViewPosition(float4 position, int ID)
{
    float4 outVal = mul(position, worldMatrix);
    outVal = mul(outVal, L_lightView[ID]);
    return mul(outVal, L_lightProjection[ID]);
}

//! CALCULATE NORMAL FROM PLANE ----------------------------------------------------------------------------------------------
//! calculates the normal from 2 delta Y each in their own plane
float3 calculateNormalFromPlane(float step, float deltaYx, float deltaYz)
{
    const float3 line1 = float3(step * 2, deltaYx, 0.f);
    const float3 line2 = float3(0.f, deltaYz, step * 2);
    return normalize(cross(-line1, line2));
}