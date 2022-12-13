#include "shader_tools_ps.hlsli"

// BUFFERS //

Texture2D blur : register(t0);
Texture2D base : register(t1);
Texture2D depth : register(t2);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// FUNCTIONS //

float4 main(InputType input) : SV_TARGET
{
    //! Sample the textures 
    float4 blurCol = blur.SampleLevel(diffuseSampler, input.tex, 0);
    float4 baseCol = base.SampleLevel(diffuseSampler, input.tex, 0);
    float depthVal = depth.SampleLevel(diffuseSampler, input.tex, 0).r - 0.003;
    
    //! blend between the textures using the depth map
    return interpolateColourFromRange(baseCol, blurCol, 1.f, 0.987f, depthVal);
}