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
	float3 worldPosition : TEXCOORD1;
	float3 viewVector : TEXCOORD2;
	float4 lightViewPos[NUM_OF_LIGHTS] : TEXCOORD3;
};

// FUNCTIONS //

OutputType main(InputType input)
{
    OutputType output;

	//! Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = calculateScreenPosition(input.position);
    output.worldPosition = calculateWorldPosition(input.position);
    
	//! Calculate the position of the vertex as viewed by the light source.
    for (int i = 0; i < NUM_OF_LIGHTS; i++)
        output.lightViewPos[i] = calculateLightViewPosition(input.position, i);

    //! propagate UVs further, flip Y coords
    output.tex = flipUVsVertical(input.tex);
    
    //! calculate normals in the world space
    output.normal = calculateWorldNormal(input.normal);

    //! camera view vector calculation
    output.viewVector = calculateCameraView(output.worldPosition);
    
    return output;
}