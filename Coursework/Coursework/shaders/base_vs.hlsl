#include "shader_tools_vs.hlsli"

// BUFFERS //

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
};

// FUNCTIONS //

OutputType main(InputType input)
{
    OutputType output;

	//! Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = calculateScreenPosition(input.position);

    //! propagate UVs further, flip Y coords
    output.tex = input.tex;
    
    //! calculate normals in the world space
    output.normal = calculateWorldNormal(input.normal);
    
    return output;
}