#include "Constants.hlsli"
#include "external.hlsli"

#define MAX_VERTICES 16 
#define FACES 4

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

cbuffer TransformBuffer : register(b2)
{
    float3 scalingRangeBottom;
    float padding2;
    float3 scalingRangeTop;
    float padding3;
};

struct InputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float4 lightViewPos[NUM_OF_LIGHTS] : TEXCOORD3;
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

// STRUCTS //

static float3 offsets[2] =
{
    float3(0.5f, 0.5f, 0.5f),
    float3(0.0f, 0.5f, 0.0f)
};

//! normals are not calculate but predefined
static float3 normals[MAX_VERTICES] =
{
    float3(-1, 1, -1),
    float3(-1, 1, -1),
    float3(0, 1, 0),
    float3(-1, 0, 0),
    
    float3(0, 1, 0),
    float3(1, 0, 0),
    float3(1, 1, -1),
    float3(1, 1, -1),
    
    float3(-1, 1, 1), 
    float3(-1, 1, 1),
    float3(0, 1, 0),
    float3(0, 0, 1),
    
    float3(0, 1, 0),
    float3(0, 0, 1),
    float3(1, 1, 1),
    float3(1, 1, 1)
};

// FUNCTIONS //

[maxvertexcount(MAX_VERTICES)]
void main(point InputType input[1], inout TriangleStream<OutputType> outputStream)
{
    // SETUP //
    
    OutputType output;
    
    //! vector from vertex to camera
    float2 direction = cameraPosition.xz - input[0].worldPosition.xz;

    //! angle between vector to camera and normal vector, 
    //! +- dictated by the relative offset of x coordinate
    float angle = acos(dot(normalize(direction), normalize(input[0].normal).xz));
    angle *= direction.x > 0 ? -1 : 1;

    //! Rotation matrix around Y axis, yaw
    matrix<float, 4, 4> rotation =
    {
        cos(angle), 0, sin(angle), 0,
        0, 1, 0, 0,
        -sin(angle), 0, cos(angle), 0,
        0, 0, 0, 1
    };
    
    // GEOMETRY GENERATION // 
    
    int rngSeed = input[0].position.x + input[0].position.y;
    float3 sF = lerp(scalingRangeBottom, scalingRangeTop, nextFloat(rngSeed));
    
    //! 4 faces 
    for (int j = 0; j < FACES; j++)
    {
        //! 4 vertices per face
        for (int i = 0; i < 4; i++)
        {
            //! determine the index of the collumn of points to use, signs are dealt with later
            int index = (i / 2) % 2 ? (j % 2 ? 0 : 1) : (j % 2 ? 1 : 0);

            //! add correct directions
            float3 finalOffset = float3(offsets[index].x * (j % 2 ? 1.f : -1.f), offsets[index].y * (i % 2 ? -1.f : 1.f), offsets[index].z * ((j / 2) % 2 ? -1.f : 1.f));
            
            //! Position generation
            //! use matrices and concatentaion to better describe the rotations
            //! uses calculations to determine the proper directions of each axis offset
            matrix<float, 4, 4> translation =
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                finalOffset.x * sF.x, finalOffset.y * sF.y, finalOffset.z * sF.z, 1
            };
            
            //! concatenate matrices and multiply the input position by them
            output.position = mul(input[0].position, mul(mul(rotation, translation), inverse(rotation)));
            output.position = mul(output.position, worldMatrix);
            output.worldPosition = output.position.xyz;
            output.position = mul(output.position, viewMatrix);
            output.position = mul(output.position, projectionMatrix);
            
            //! UV generation
            output.tex = float2(finalOffset.x + 0.5f, 1 - (finalOffset.y + 0.5f));
            
            //! Normal generation
            //! use matrices and concatentaion to better describe the rotations
            matrix<float, 4, 4> normalTranslation =
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                finalOffset.x * sF.x, finalOffset.y * sF.y, finalOffset.z * sF.z, 1
            };
            
            //! takes into account the constant rotation to the camera, hence normals are not determined based on the face
            output.normal = mul(float4(normals[i], 1.f), rotation).xyz;
            output.normal = mul(output.normal, (float3x3) worldMatrix);
            output.normal = normalize(output.normal);
            
            //! passing the other parameters to pixel shader
            output.lightViewPos = input[0].lightViewPos;
            output.viewVector = input[0].viewVector.xyz;
            
            //! add the generated vertex to stream
            outputStream.Append(output);
        }
        //! start generating the next face
        outputStream.RestartStrip();
    }
}

