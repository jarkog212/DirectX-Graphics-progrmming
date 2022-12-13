#include "shader_tools_vs.hlsli"

// BUFFERS //

struct InputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : POSITION;
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
    output.position = input.position;
    output.worldPosition = calculateWorldPosition(input.position);
    
	//! Calculate the position of the vertice as viewed by the light source.
    for (int i = 0; i < NUM_OF_LIGHTS; i++)
        output.lightViewPos[i] = calculateLightViewPosition(input.position, i);
    
    //! calculate the normals in the world space
    output.normal = input.normal;

    //! View vector per vertex
    output.viewVector = calculateCameraView(output.worldPosition);
    
    return output;
}