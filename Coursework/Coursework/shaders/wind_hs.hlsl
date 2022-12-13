// Tessellation Hull Shader
// Prepares control points for tessellation

// BUFFERS //

cbuffer HullShaderBuffer : register(b0)
{
    float3 cameraPos;
    float maxTesselationFactor;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct OutputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// FUNCTIONS // 

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

    //! Set the tessellation factors for the three edges of the triangle.
    float distance = abs(length(cameraPos - inputPatch[0].worldPosition));
    
    //! These consts could be in the buffer and modifiable
    const uint maxLODs = 5;
    const float maxDistance = 100;
    
    //! calculate factor based on camera distance 
    float section = maxDistance / maxLODs;
    float LOD = clamp(min(distance, maxDistance) / section, 1, maxLODs);
    uint factor = clamp(maxTesselationFactor * (1.f / LOD), 1, maxTesselationFactor);
    
    output.edges[0] = factor; 
    output.edges[1] = factor; 
    output.edges[2] = factor;

    //! Set the tessellation factor for tessallating inside the triangle.
    output.inside = factor; //inside.x;
    
    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 3> patch, uint pointId : SV_OutputControlPointID)
{
    OutputType output;

    //! propagate position.
    output.position = patch[pointId].position;

    //! propagate UVs.
    output.tex = patch[pointId].tex;

    //! propagate normals
    output.normal = patch[pointId].normal;
    
    return output;
}