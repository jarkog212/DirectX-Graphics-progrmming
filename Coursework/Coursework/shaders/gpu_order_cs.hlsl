#include "Constants.hlsli"
#include "external.hlsli"

#define MAX_NUM_TO_SORT 5000
#define GROUP_WIDTH 1000

// STRUCTS //

struct VertexType
{
    float3 position;
    float p;
};

struct VertexContainer
{
    float ID;
    float distance;
    float2 p2;
};

struct ComputeBufferType
{
    VertexType vertex;
    VertexContainer unordered;
};

// BUFFERS //

cbuffer ComputeConstantsBuffer : register(b0)
{
    float numOfPoints;
    float3 cameraPosition;
    float numOfPasses;
    float3 p;
};

//! separated due to memory limitations
cbuffer VertexListBuffer : register(b1)
{
    VertexType list[MAX_NUM_TO_SORT / 4];
};

cbuffer VertexListBuffer : register(b2)
{
    VertexType list2[MAX_NUM_TO_SORT / 4];
};

cbuffer VertexListBuffer : register(b3)
{
    VertexType list3[MAX_NUM_TO_SORT / 4];
};

cbuffer VertexListBuffer : register(b4)
{
    VertexType list4[MAX_NUM_TO_SORT / 4];
};

RWStructuredBuffer<ComputeBufferType> computeBuffer : register(u0);

// FUNCTIONS //

//! use single index to correctly fetch data from the right buffer
VertexType getItemFromBuffers(uint index)
{
    int quarter = MAX_NUM_TO_SORT / 4;
    
    if (index < quarter)
        return list[index];
    else if (index < quarter * 2)
        return list2[index - quarter];
    else if (index < quarter * 3)
        return list3[index - (quarter * 2)];
    else
        return list4[index - (quarter * 3)];
}

void swapElements(int index, int index2)
{
    VertexContainer temp = computeBuffer[index].unordered;
    computeBuffer[index].unordered = computeBuffer[index2].unordered;
    computeBuffer[index2].unordered = temp;

}

[numthreads(GROUP_WIDTH, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
{
    //! figure out the index of the vertex this thread will deal with and store the vertex to limit the number of accesses
    uint index = dispatchThreadID.x; //(groupID.x * 1000) + groupThreadID.x;
    
    //! calculate distance of each point from camera, if no point then set the distance to -1
    if (index < min(MAX_NUM_TO_SORT, numOfPoints))
        computeBuffer[index].unordered.distance = abs(length(getItemFromBuffers(index).position - cameraPosition));
    else
        computeBuffer[index].unordered.distance = -1.f;
    
    //! store the original ID for final sorting
    computeBuffer[index].unordered.ID = index;
    
    AllMemoryBarrierWithGroupSync();
   
    //! uses odd-even sorting, pairs get sorted in between eachother until no swaps occur, then its sorted
    //! sorting in descending order, furthest trees are drawn earlier than closer ones

    for (int i = 0; i < numOfPasses; i++)
    {
    //EVEN phase
        if (!(index % 2) && !(index + 1 > min(MAX_NUM_TO_SORT, numOfPoints)) && groupThreadID.x < GROUP_WIDTH - 1)
            if (computeBuffer[index].unordered.distance < computeBuffer[index + 1].unordered.distance)
                swapElements(index, index + 1);

        AllMemoryBarrierWithGroupSync();

    //ODD phase
        if (index % 2 && !(index + 1 > min(MAX_NUM_TO_SORT, numOfPoints)) && groupThreadID.x < GROUP_WIDTH - 1)
            if (computeBuffer[index].unordered.distance < computeBuffer[index + 1].unordered.distance)
                swapElements(index, index + 1);
    }
    
    AllMemoryBarrierWithGroupSync();
 
    
    //! copy the Vertex to ensure no race-conditions can occur when placing it back
    if (index < min(MAX_NUM_TO_SORT, numOfPoints))
        computeBuffer[index].vertex = getItemFromBuffers(computeBuffer[index].unordered.ID);
    
    AllMemoryBarrierWithGroupSync();
    
    return;
};