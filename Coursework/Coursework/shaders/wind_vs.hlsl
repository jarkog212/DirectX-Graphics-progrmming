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
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
};

// FUNCTIONS //

OutputType main(InputType input)
{
    OutputType _out;
    
    //! propagate the initial data of the vertex
    _out.position = input.position;
    _out.normal = input.normal;
    _out.tex = input.tex;
    
    //! calculate world position for dynamic tesselation
    _out.worldPosition = calculateWorldPosition(_out.position);
    
     return _out;
}