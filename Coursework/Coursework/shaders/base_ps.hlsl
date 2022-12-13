
// BUFFERS //

Texture2D diffuse : register(t0);
sampler diffuseSampler : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// FUNCTIONS //

float4 main(InputType input) : SV_TARGET
{
    return diffuse.Sample(diffuseSampler, input.tex);
}