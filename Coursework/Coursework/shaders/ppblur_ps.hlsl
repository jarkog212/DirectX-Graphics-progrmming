// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry

// BUFFERS //

// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer PPBoxBlurBuffer : register(b0)
{
	float2 resolution;
	float2 dimensions;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

// FUNCTIONS //

float4 main(InputType input) : SV_TARGET
{
	//! texel dimensions from resolution
    const float texelWidth = (1.00f / resolution.x) * 3;
    const float texelHeight = (1.00f / resolution.y) * 3;
	
    //! doesn't start at pixel location but in the "top right" corner of the area of the blur
    float2 originSamplingPosition = { -(dimensions.x / 2.f) * texelWidth, -(dimensions.y / 2.f) * texelHeight };
	
    //! perform blur 
    float4 textureColor = float4(0,0,0,0);
    for (int i = 0; i < dimensions.y; i++)
        for (int j = 0; j < dimensions.x; j++)
            textureColor += texture0.Sample(Sampler0, input.tex + float2(originSamplingPosition.x + texelWidth * j, originSamplingPosition.y + texelHeight * i));
	
	//! average the colour of the 9 pixels (all have the same weight)
    textureColor /= (dimensions.x * dimensions.y);

	//! set alpha to 1
	textureColor.a = 1.00f;

	return textureColor;
}