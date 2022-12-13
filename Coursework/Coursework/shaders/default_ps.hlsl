#include "shader_tools_ps.hlsli"

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : TEXCOORD1;
	float3 viewVector : TEXCOORD2;
	float4 lightViewPos[NUM_OF_LIGHTS] : TEXCOORD3; 
};

float4 main(InputType input) : SV_TARGET
{
//----------------NORMAL MAP-----------------
	
    float3 normalVector = handleNormalMap(input.normal, input.tex);
    
//----------------LIGHTS AND SHADOWS---------
	
    Light_Colours lightData = handleLights(
        input.lightViewPos,
        normalVector,
        input.worldPosition,
        input.viewVector,
        input.tex);
 
    //! handle metallic, requires reflecions, stretch goal
	//! handle proper rougness, stretch goal - hard as that will require reflections, reflect at certain angle opposite to camera
    
    return assembleFinalShade(input.tex, lightData.shadowPasses, lightData.lightColour, lightData.ambient, lightData.specular);
    //! debug for normals
    //return float4(normalVector.x / 2 + .5f, normalVector.y / 2 + .5f, normalVector.z / 2 + .5f, 1);

}