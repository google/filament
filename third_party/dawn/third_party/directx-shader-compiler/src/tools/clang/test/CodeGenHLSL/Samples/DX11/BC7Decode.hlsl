// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: groupId
// CHECK: bufferLoad
// CHECK: bufferStore


//--------------------------------------------------------------------------------------
// File: BC7Encode.hlsl
//
// The Compute Shader for BC7 Decoder
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//#define REF_DEVICE

#define UINTLENGTH			32
#define NCHANNELS			4
#define BC7_UNORM			98

static const uint candidateSectionCompressed[128] = 
{
    0x5050505, 0x1010101, 0x15151515, 0x1050515, 
    0x10105, 0x5151555, 0x1051555, 0x10515, 
    0x105, 0x5155555, 0x11555, 0x115, 
    0x1155555, 0x5555, 0x555555, 0x55, 
    0x405455, 0x15010000, 0x4054, 0x15050100, 
    0x5010000, 0x405054, 0x4050, 0x15050501, 
    0x5010100, 0x404050, 0x14141414, 0x5141450, 
    0x1155440, 0x555500, 0x15014054, 0x5414150, 

    0x11111111, 0x550055, 0x11441144, 0x5055050, 
    0x5500550, 0x11114444, 0x14411441, 0x11444411, 
    0x15055054, 0x1055040, 0x5041050, 0x5455150, 
    0x14414114, 0x5505005, 0x14144141, 0x141400, 
    0x10541000, 0x4150400, 0x41504, 0x105410, 
    0x14504105, 0x5145041, 0x14054150, 0x5415014, 
    0x14505041, 0x14050541, 0x15544001, 0x1405415, 
    0x550505, 0x5055500, 0x4045454, 0x10101515, 

    0x50529aa, 0x105a5a9, 0x81a5a5, 0x2a0a0515, 
    0x5a5a, 0x5050a0a, 0xa0a5555, 0x505a5a5, 
    0x55aa, 0x5555aa, 0x55aaaa, 0x6060606, 
    0x16161616, 0x1a1a1a1a, 0x5165a6a, 0x581a0a8, 
    0x105165a, 0x150581a0, 0x5a5a5a, 0xa0a0a55, 
    0x15152a2a, 0x101a9a9, 0x51a1a, 0x50a4a4, 
    0x1a1a0500, 0x6065aaa, 0x14696914, 0x146969, 
    0xa52520a, 0x141482aa, 0x51a1a05, 0x80a5a9, 

    0x25a6a, 0x2a0a0605, 0x5060a2a, 0x18181818, 
    0x55aa00, 0x18618618, 0x18866118, 0x5a05a05, 
    0x55aa005, 0x1111aaaa, 0x9999, 0xa5a0a5a, 
    0xa050a05, 0x28692869, 0x11aaaa11, 0x999999, 
    0x111111aa, 0x2a152a15, 0x2560256, 0x969696, 
    0x2a15152a, 0x2565602, 0x141414aa, 0x9696, 
    0x1414aaaa, 0xa05050a, 0xa5a5a0a, 0x96, 
    0x2010201, 0x2a6a2a6a, 0x11aaaaaa, 0x1585a1a8
};	
/*static const uint4x4 candidateSection[128] = 
{
    {0,0,1,1, 0,0,1,1, 0,0,1,1, 0,0,1,1}, {0,0,0,1, 0,0,0,1, 0,0,0,1, 0,0,0,1}, {0,1,1,1, 0,1,1,1, 0,1,1,1, 0,1,1,1}, {0,0,0,1, 0,0,1,1, 0,0,1,1, 0,1,1,1},
    {0,0,0,0, 0,0,0,1, 0,0,0,1, 0,0,1,1}, {0,0,1,1, 0,1,1,1, 0,1,1,1, 1,1,1,1}, {0,0,0,1, 0,0,1,1, 0,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,1, 0,0,1,1, 0,1,1,1},
    {0,0,0,0, 0,0,0,0, 0,0,0,1, 0,0,1,1}, {0,0,1,1, 0,1,1,1, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,1, 0,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,0, 0,0,0,1, 0,1,1,1},
    {0,0,0,1, 0,1,1,1, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,0, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 1,1,1,1, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,0, 0,0,0,0, 1,1,1,1},
    {0,0,0,0, 1,0,0,0, 1,1,1,0, 1,1,1,1}, {0,1,1,1, 0,0,0,1, 0,0,0,0, 0,0,0,0}, {0,0,0,0, 0,0,0,0, 1,0,0,0, 1,1,1,0}, {0,1,1,1, 0,0,1,1, 0,0,0,1, 0,0,0,0},
    {0,0,1,1, 0,0,0,1, 0,0,0,0, 0,0,0,0}, {0,0,0,0, 1,0,0,0, 1,1,0,0, 1,1,1,0}, {0,0,0,0, 0,0,0,0, 1,0,0,0, 1,1,0,0}, {0,1,1,1, 0,0,1,1, 0,0,1,1, 0,0,0,1},
    {0,0,1,1, 0,0,0,1, 0,0,0,1, 0,0,0,0}, {0,0,0,0, 1,0,0,0, 1,0,0,0, 1,1,0,0}, {0,1,1,0, 0,1,1,0, 0,1,1,0, 0,1,1,0}, {0,0,1,1, 0,1,1,0, 0,1,1,0, 1,1,0,0},
    {0,0,0,1, 0,1,1,1, 1,1,1,0, 1,0,0,0}, {0,0,0,0, 1,1,1,1, 1,1,1,1, 0,0,0,0}, {0,1,1,1, 0,0,0,1, 1,0,0,0, 1,1,1,0}, {0,0,1,1, 1,0,0,1, 1,0,0,1, 1,1,0,0},
    
    {0,1,0,1, 0,1,0,1, 0,1,0,1, 0,1,0,1}, {0,0,0,0, 1,1,1,1, 0,0,0,0, 1,1,1,1}, {0,1,0,1, 1,0,1,0, 0,1,0,1, 1,0,1,0}, {0,0,1,1, 0,0,1,1, 1,1,0,0, 1,1,0,0},
    {0,0,1,1, 1,1,0,0, 0,0,1,1, 1,1,0,0}, {0,1,0,1, 0,1,0,1, 1,0,1,0, 1,0,1,0}, {0,1,1,0, 1,0,0,1, 0,1,1,0, 1,0,0,1}, {0,1,0,1, 1,0,1,0, 1,0,1,0, 0,1,0,1},
    {0,1,1,1, 0,0,1,1, 1,1,0,0, 1,1,1,0}, {0,0,0,1, 0,0,1,1, 1,1,0,0, 1,0,0,0}, {0,0,1,1, 0,0,1,0, 0,1,0,0, 1,1,0,0}, {0,0,1,1, 1,0,1,1, 1,1,0,1, 1,1,0,0},
    {0,1,1,0, 1,0,0,1, 1,0,0,1, 0,1,1,0}, {0,0,1,1, 1,1,0,0, 1,1,0,0, 0,0,1,1}, {0,1,1,0, 0,1,1,0, 1,0,0,1, 1,0,0,1}, {0,0,0,0, 0,1,1,0, 0,1,1,0, 0,0,0,0},
    {0,1,0,0, 1,1,1,0, 0,1,0,0, 0,0,0,0}, {0,0,1,0, 0,1,1,1, 0,0,1,0, 0,0,0,0}, {0,0,0,0, 0,0,1,0, 0,1,1,1, 0,0,1,0}, {0,0,0,0, 0,1,0,0, 1,1,1,0, 0,1,0,0},
    {0,1,1,0, 1,1,0,0, 1,0,0,1, 0,0,1,1}, {0,0,1,1, 0,1,1,0, 1,1,0,0, 1,0,0,1}, {0,1,1,0, 0,0,1,1, 1,0,0,1, 1,1,0,0}, {0,0,1,1, 1,0,0,1, 1,1,0,0, 0,1,1,0},
    {0,1,1,0, 1,1,0,0, 1,1,0,0, 1,0,0,1}, {0,1,1,0, 0,0,1,1, 0,0,1,1, 1,0,0,1}, {0,1,1,1, 1,1,1,0, 1,0,0,0, 0,0,0,1}, {0,0,0,1, 1,0,0,0, 1,1,1,0, 0,1,1,1},
    {0,0,0,0, 1,1,1,1, 0,0,1,1, 0,0,1,1}, {0,0,1,1, 0,0,1,1, 1,1,1,1, 0,0,0,0}, {0,0,1,0, 0,0,1,0, 1,1,1,0, 1,1,1,0}, {0,1,0,0, 0,1,0,0, 0,1,1,1, 0,1,1,1},
    
    {0,0,1,1, 0,0,1,1, 0,2,2,1, 2,2,2,2}, {0,0,0,1, 0,0,1,1, 2,2,1,1, 2,2,2,1}, {0,0,0,0, 2,0,0,1, 2,2,1,1, 2,2,1,1}, {0,2,2,2, 0,0,2,2, 0,0,1,1, 0,1,1,1},
    {0,0,0,0, 0,0,0,0, 1,1,2,2, 1,1,2,2}, {0,0,1,1, 0,0,1,1, 0,0,2,2, 0,0,2,2}, {0,0,2,2, 0,0,2,2, 1,1,1,1, 1,1,1,1}, {0,0,1,1, 0,0,1,1, 2,2,1,1, 2,2,1,1},
    {0,0,0,0, 0,0,0,0, 1,1,1,1, 2,2,2,2}, {0,0,0,0, 1,1,1,1, 1,1,1,1, 2,2,2,2}, {0,0,0,0, 1,1,1,1, 2,2,2,2, 2,2,2,2}, {0,0,1,2, 0,0,1,2, 0,0,1,2, 0,0,1,2},
    {0,1,1,2, 0,1,1,2, 0,1,1,2, 0,1,1,2}, {0,1,2,2, 0,1,2,2, 0,1,2,2, 0,1,2,2}, {0,0,1,1, 0,1,1,2, 1,1,2,2, 1,2,2,2}, {0,0,1,1, 2,0,0,1, 2,2,0,0, 2,2,2,0},
    {0,0,0,1, 0,0,1,1, 0,1,1,2, 1,1,2,2}, {0,1,1,1, 0,0,1,1, 2,0,0,1, 2,2,0,0}, {0,0,0,0, 1,1,2,2, 1,1,2,2, 1,1,2,2}, {0,0,2,2, 0,0,2,2, 0,0,2,2, 1,1,1,1},
    {0,1,1,1, 0,1,1,1, 0,2,2,2, 0,2,2,2}, {0,0,0,1, 0,0,0,1, 2,2,2,1, 2,2,2,1}, {0,0,0,0, 0,0,1,1, 0,1,2,2, 0,1,2,2}, {0,0,0,0, 1,1,0,0, 2,2,1,0, 2,2,1,0},
    {0,1,2,2, 0,1,2,2, 0,0,1,1, 0,0,0,0}, {0,0,1,2, 0,0,1,2, 1,1,2,2, 2,2,2,2}, {0,1,1,0, 1,2,2,1, 1,2,2,1, 0,1,1,0}, {0,0,0,0, 0,1,1,0, 1,2,2,1, 1,2,2,1},
    {0,0,2,2, 1,1,0,2, 1,1,0,2, 0,0,2,2}, {0,1,1,0, 0,1,1,0, 2,0,0,2, 2,2,2,2}, {0,0,1,1, 0,1,2,2, 0,1,2,2, 0,0,1,1}, {0,0,0,0, 2,0,0,0, 2,2,1,1, 2,2,2,1},
    
    {0,0,0,0, 0,0,0,2, 1,1,2,2, 1,2,2,2}, {0,2,2,2, 0,0,2,2, 0,0,1,2, 0,0,1,1}, {0,0,1,1, 0,0,1,2, 0,0,2,2, 0,2,2,2}, {0,1,2,0, 0,1,2,0, 0,1,2,0, 0,1,2,0},
    {0,0,0,0, 1,1,1,1, 2,2,2,2, 0,0,0,0}, {0,1,2,0, 1,2,0,1, 2,0,1,2, 0,1,2,0}, {0,1,2,0, 2,0,1,2, 1,2,0,1, 0,1,2,0}, {0,0,1,1, 2,2,0,0, 1,1,2,2, 0,0,1,1},
    {0,0,1,1, 1,1,2,2, 2,2,0,0, 0,0,1,1}, {0,1,0,1, 0,1,0,1, 2,2,2,2, 2,2,2,2}, {0,0,0,0, 0,0,0,0, 2,1,2,1, 2,1,2,1}, {0,0,2,2, 1,1,2,2, 0,0,2,2, 1,1,2,2},
    {0,0,2,2, 0,0,1,1, 0,0,2,2, 0,0,1,1}, {0,2,2,0, 1,2,2,1, 0,2,2,0, 1,2,2,1}, {0,1,0,1, 2,2,2,2, 2,2,2,2, 0,1,0,1}, {0,0,0,0, 2,1,2,1, 2,1,2,1, 2,1,2,1},
    {0,1,0,1, 0,1,0,1, 0,1,0,1, 2,2,2,2}, {0,2,2,2, 0,1,1,1, 0,2,2,2, 0,1,1,1}, {0,0,0,2, 1,1,1,2, 0,0,0,2, 1,1,1,2}, {0,0,0,0, 2,1,1,2, 2,1,1,2, 2,1,1,2},
    {0,2,2,2, 0,1,1,1, 0,1,1,1, 0,2,2,2}, {0,0,0,2, 1,1,1,2, 1,1,1,2, 0,0,0,2}, {0,1,1,0, 0,1,1,0, 0,1,1,0, 2,2,2,2}, {0,0,0,0, 0,0,0,0, 2,1,1,2, 2,1,1,2},
    {0,1,1,0, 0,1,1,0, 2,2,2,2, 2,2,2,2}, {0,0,2,2, 0,0,1,1, 0,0,1,1, 0,0,2,2}, {0,0,2,2, 1,1,2,2, 1,1,2,2, 0,0,2,2}, {0,0,0,0, 0,0,0,0, 0,0,0,0, 2,1,1,2},
    {0,0,0,2, 0,0,0,1, 0,0,0,2, 0,0,0,1}, {0,2,2,2, 1,2,2,2, 0,2,2,2, 1,2,2,2}, {0,1,0,1, 2,2,2,2, 2,2,2,2, 2,2,2,2}, {0,1,1,1, 2,0,1,1, 2,2,0,1, 2,2,2,0}
};*/
/*static const uint2 candidateFixUpIndex1D[128] = 
{
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{ 2, 0},{ 8, 0},{ 2, 0},
    { 2, 0},{ 8, 0},{ 8, 0},{15, 0},
    { 2, 0},{ 8, 0},{ 2, 0},{ 2, 0},
    { 8, 0},{ 8, 0},{ 2, 0},{ 2, 0},
    
    {15, 0},{15, 0},{ 6, 0},{ 8, 0},
    { 2, 0},{ 8, 0},{15, 0},{15, 0},
    { 2, 0},{ 8, 0},{ 2, 0},{ 2, 0},
    { 2, 0},{15, 0},{15, 0},{ 6, 0},
    { 6, 0},{ 2, 0},{ 6, 0},{ 8, 0},
    {15, 0},{15, 0},{ 2, 0},{ 2, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{ 2, 0},{ 2, 0},{15, 0},
    //candidateFixUpIndex1D[i][1], i < 64 should not be used
    
    { 3,15},{ 3, 8},{15, 8},{15, 3},
    { 8,15},{ 3,15},{15, 3},{15, 8},
    { 8,15},{ 8,15},{ 6,15},{ 6,15},
    { 6,15},{ 5,15},{ 3,15},{ 3, 8},
    { 3,15},{ 3, 8},{ 8,15},{15, 3},
    { 3,15},{ 3, 8},{ 6,15},{10, 8},
    { 5, 3},{ 8,15},{ 8, 6},{ 6,10},
    { 8,15},{ 5,15},{15,10},{15, 8},
    
    { 8,15},{15, 3},{ 3,15},{ 5,10},
    { 6,10},{10, 8},{ 8, 9},{15,10},
    {15, 6},{ 3,15},{15, 8},{ 5,15},
    {15, 3},{15, 6},{15, 6},{15, 8}, //The Spec doesn't mark the first fixed up index in this row, so I apply 15 for them, and seems correct
    { 3,15},{15, 3},{ 5,15},{ 5,15},
    { 5,15},{ 8,15},{ 5,15},{10,15},
    { 5,15},{10,15},{ 8,15},{13,15},
    {15, 3},{12,15},{ 3,15},{ 3, 8},
};*/
static const uint2 candidateFixUpIndex1DOrdered[128] = //Same with candidateFixUpIndex1D but order the result when i >= 64
{
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{ 2, 0},{ 8, 0},{ 2, 0},
    { 2, 0},{ 8, 0},{ 8, 0},{15, 0},
    { 2, 0},{ 8, 0},{ 2, 0},{ 2, 0},
    { 8, 0},{ 8, 0},{ 2, 0},{ 2, 0},
    
    {15, 0},{15, 0},{ 6, 0},{ 8, 0},
    { 2, 0},{ 8, 0},{15, 0},{15, 0},
    { 2, 0},{ 8, 0},{ 2, 0},{ 2, 0},
    { 2, 0},{15, 0},{15, 0},{ 6, 0},
    { 6, 0},{ 2, 0},{ 6, 0},{ 8, 0},
    {15, 0},{15, 0},{ 2, 0},{ 2, 0},
    {15, 0},{15, 0},{15, 0},{15, 0},
    {15, 0},{ 2, 0},{ 2, 0},{15, 0},
    //candidateFixUpIndex1DOrdered[i][1], i < 64 should not be used
    
    { 3,15},{ 3, 8},{ 8,15},{ 3,15},
    { 8,15},{ 3,15},{ 3,15},{ 8,15},
    { 8,15},{ 8,15},{ 6,15},{ 6,15},
    { 6,15},{ 5,15},{ 3,15},{ 3, 8},
    { 3,15},{ 3, 8},{ 8,15},{ 3,15},
    { 3,15},{ 3, 8},{ 6,15},{ 8,10},
    { 3, 5},{ 8,15},{ 6, 8},{ 6,10},
    { 8,15},{ 5,15},{10,15},{ 8,15},
    
    { 8,15},{ 3,15},{ 3,15},{ 5,10},
    { 6,10},{ 8,10},{ 8, 9},{10,15},
    { 6,15},{ 3,15},{ 8,15},{ 5,15},
    { 3,15},{ 6,15},{ 6,15},{ 8,15}, //The Spec doesn't mark the first fixed up index in this row, so I apply 15 for them
    { 3,15},{ 3,15},{ 5,15},{ 5,15},
    { 5,15},{ 8,15},{ 5,15},{10,15},
    { 5,15},{10,15},{ 8,15},{13,15},
    { 3,15},{12,15},{ 3,15},{ 3, 8},
};
static const uint4x4 candidateRotation[4] = 
{
    {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1},
    {0,0,0,1},{0,1,0,0},{0,0,1,0},{1,0,0,0},
    {1,0,0,0},{0,0,0,1},{0,0,1,0},{0,1,0,0},
    {1,0,0,0},{0,1,0,0},{0,0,0,1},{0,0,1,0}
};
static const uint2 candidateIndexPrec[8] = {{3,0},{3,0},{2,0},{2,0},
                                            {2,3}, //color index and alpha index can exchange
                                            {2,2},{4,4},{2,2}};


/*cbuffer cbCS : register( b0 )
{
    uint4 g_param;	//(g_param.x, g_param.y) is the x and y dimensions of the Dispatch call
                    //g_param.z defines the format, should be only BC7_UNORM, but is not used in the shader
};*/
cbuffer cbCS : register( b0 )
{
    uint g_tex_width;
    uint g_num_block_x;
    uint g_format;
    uint g_tex_size;
    uint g_start_block_id;
};

void extract_mode_and_partition( out uint mode, out uint partition,	out uint rotation, out uint2 indexPrec, uint4 block );
void extract_and_decode_endpoints_00( out uint4 endPoint, uint mode, uint4 block );
void extract_and_decode_endpoints_01( out uint4 endPoint, uint mode, uint4 block );
void extract_and_decode_endpoints_10( out uint4 endPoint, uint mode, uint4 block );
void extract_and_decode_endpoints_11( out uint4 endPoint, uint mode, uint4 block );
void extract_and_decode_endpoints_20( out uint4 endPoint, uint mode, uint4 block );
void extract_and_decode_endpoints_21( out uint4 endPoint, uint mode, uint4 block );
void get_index( out uint alpha_index, out uint color_index, out uint subset_index, uint x, uint y, uint mode, uint partition, uint4 block );
uint3 interpolate_color( uint color_index, uint index_prec, uint2x4 endPoint );
uint interpolate_alpha( uint alpha_index, uint index_prec, uint2x4 endPoint );

StructuredBuffer<uint4> g_InBuff : register( t0 );
RWStructuredBuffer<uint> g_OutBuff : register( u0 );

#define THREAD_GROUP_SIZE	64
#define BLOCK_SIZE_Y		4
#define BLOCK_SIZE_X		4
#define BLOCK_SIZE			(BLOCK_SIZE_Y * BLOCK_SIZE_X)
#define BLOCK_IN_GROUP		(THREAD_GROUP_SIZE / BLOCK_SIZE)

groupshared uint4 shared_temp[THREAD_GROUP_SIZE];

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void main(uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID)
{
    uint blockInGroup = GI / BLOCK_SIZE;
    uint blockID = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup;
    uint threadInBlock = GI - blockInGroup * BLOCK_SIZE;

    if (0 == threadInBlock)
    {
        shared_temp[GI] = g_InBuff[blockID];
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    uint4 bc_data = shared_temp[blockInGroup * BLOCK_SIZE + 0];
    
    uint mode;
    uint partition;
    uint rotation;
    uint2 indexPrec;
    extract_mode_and_partition( mode, partition, rotation, indexPrec, bc_data );

    if (1 == threadInBlock)
    {
        uint4 endPoint;
        extract_and_decode_endpoints_00( endPoint, mode, bc_data );
        shared_temp[GI] = endPoint;
    }
    else if (2 == threadInBlock)
    {
        uint4 endPoint;
        extract_and_decode_endpoints_01( endPoint, mode, bc_data );
        shared_temp[GI] = endPoint;
    }
    else if (3 == threadInBlock)
    {
        uint4 endPoint;
        extract_and_decode_endpoints_10( endPoint, mode, bc_data );
        shared_temp[GI] = endPoint;
    }
    else if (4 == threadInBlock)
    {
        uint4 endPoint;
        extract_and_decode_endpoints_11( endPoint, mode, bc_data );
        shared_temp[GI] = endPoint;
    }
    else if (5 == threadInBlock)
    {
        uint4 endPoint;
        extract_and_decode_endpoints_20( endPoint, mode, bc_data );
        shared_temp[GI] = endPoint;
    }
    else if (6 == threadInBlock)
    {
        uint4 endPoint;
        extract_and_decode_endpoints_21( endPoint, mode, bc_data );
        shared_temp[GI] = endPoint;
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    uint y = threadInBlock / BLOCK_SIZE_X;
    uint x = threadInBlock - y * BLOCK_SIZE_X;

    uint block_y = blockID / g_num_block_x;
    uint block_x = blockID - block_y * g_num_block_x;
    uint addr = (block_y * BLOCK_SIZE_Y + y) * g_tex_width + block_x * BLOCK_SIZE_X + x;
    if (addr < g_tex_size)
    {
        uint alpha_index = 0;
        uint color_index = 0;
        uint subset_index = 0;
        get_index( alpha_index, color_index, subset_index, x, y, mode, partition, bc_data );

        uint2x4 endPoint; //At most has 3 pairs of endpoints
        endPoint[0] = shared_temp[blockInGroup * BLOCK_SIZE + subset_index * 2 + 1];
        endPoint[1] = shared_temp[blockInGroup * BLOCK_SIZE + subset_index * 2 + 2];

        uint4 pixel;
        pixel.rgb = interpolate_color( color_index, indexPrec.x, endPoint);
        if ( mode >= 4 )
        {
            pixel.a = interpolate_alpha( alpha_index, indexPrec.y, endPoint);
        }
        else
        {
            pixel.a = 255;
        }

        uint4 pixelFinal = pixel;
        if (1 == rotation)
        {
            pixelFinal.ra = pixel.ar;
        }
        else if (2 == rotation)
        {
            pixelFinal.ga = pixel.ag;
        }
        else if (3 == rotation)
        {
            pixelFinal.ba = pixel.ab;
        }

        g_OutBuff[addr] = pixelFinal.r | ( pixelFinal.g <<  8 ) | ( pixelFinal.b << 16 ) | ( pixelFinal.a << 24 );
    }
}

void extract_mode_and_partition( out uint mode, out uint partition,	out uint rotation, out uint2 indexPrec, uint4 block )
{
    if ( block.x & 0x01 )
    {
        mode = 0;
        partition = ( ( block.x >> 1 ) & 0x0F ) + 64;
        rotation = 0;
        indexPrec = candidateIndexPrec[0];
    }
    else if ( block.x & 0x02 )
    {
        mode = 1;
        partition = ( block.x >> 2 ) & 0x3F;
        rotation = 0;
        indexPrec = candidateIndexPrec[1];
    }
    else if ( block.x & 0x04 )
    {
        mode = 2;
        partition = ( ( block.x >> 3 ) & 0x3F ) + 64;
        rotation = 0;
        indexPrec = candidateIndexPrec[2];
    }
    else if ( block.x & 0x08 )
    {
        mode = 3;
        partition = ( block.x >> 4 ) & 0x3F;
        rotation = 0;
        indexPrec = candidateIndexPrec[3];
    }
    else if ( block.x & 0x10 )
    {
        mode = 4;
        partition = 0;
        rotation = ( block.x >> 5 ) & 0x03;
        if ( block.x & 0x80 )
            indexPrec = uint2( 3, 2 );
        else
            indexPrec = candidateIndexPrec[4];
    }
    else if ( block.x & 0x20 )
    {
        mode = 5;
        partition = 0;
        rotation = ( block.x >> 6 ) & 0x03;
        indexPrec = candidateIndexPrec[5];
    }
    else if ( block.x & 0x40 )
    {
        mode = 6;
        partition = 0;
        rotation = 0;
        indexPrec = candidateIndexPrec[6];
    }
    else //block.x & 0x80
    {
        mode = 7;
        partition = ( block.x >> 8 ) & 0x3F;
        rotation = 0;
        indexPrec = candidateIndexPrec[7];
    }
}

void extract_and_decode_endpoints_00( out uint4 endPoint, uint mode, uint4 block )
{
    if ( mode == 0 )
    {
        endPoint.r = ( block.x >>  1 ) & 0xF0;
        endPoint.g = ( ( block.x >> 25 ) & 0xF0 ) | ( ( block.y <<  7 ) & 0xF0 );
        endPoint.b = ( block.y >> 17 ) & 0xF0;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 10 ) & 0x08 ) | ( endPoint.rgb >>  5 );
    }
    else if ( mode == 1 )
    {
        endPoint.r = ( block.x >>  6 ) & 0xFC;
        endPoint.g = ( block.y <<  2 ) & 0xFC;
        endPoint.b = ( block.y >> 22 ) & 0xFC;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 15 ) & 0x02 ) | ( endPoint.rgb >>  7 );
    }
    else if ( mode == 2 )
    {
        endPoint.r = ( block.x >>  6 ) & 0xF8;
        endPoint.g = ( block.y >>  4 ) & 0xF8;
        endPoint.b = ( block.z >>  2 ) & 0xF8;
        endPoint.a = 255;
        endPoint |= endPoint >>  5;
    }
    else if ( mode == 3 )
    {
        endPoint.r = ( block.x >>  9 ) & 0xFE;
        endPoint.g = ( block.y >>  5 ) & 0xFE;
        endPoint.b = ( block.z >>  1 ) & 0xFE;
        endPoint.a = 255;
        endPoint.rgb |= ( block.z >> 30 ) & 0x01;
    }
    else if ( mode == 4 )
    {
        endPoint.r = ( block.x >>  5 ) & 0xF8;
        endPoint.g = ( block.x >> 15 ) & 0xF8;
        endPoint.b = ( ( block.x >> 25 ) & 0xF8 ) | ( ( block.y <<  7 ) & 0xF8 );
        endPoint.a = ( block.y >>  4 ) & 0xFC;
        endPoint.rgb |= endPoint.rgb >>  5;
        endPoint.a |= endPoint.a >>  6;
    }
    else if ( mode == 5 )
    {
        endPoint.r = ( block.x >>  7 ) & 0xFE;
        endPoint.g = ( block.x >> 21 ) & 0xFE;
        endPoint.b = ( block.y >>  3 ) & 0xFE;
        endPoint.a = ( block.y >> 18 ) & 0xFF;
        endPoint.rgb |= endPoint.rgb >>  7;
    }
    else if ( mode == 6 )
    {
        endPoint.r = ( block.x >>  6 ) & 0xFE;
        endPoint.g = ( block.x >> 20 ) & 0xFE;
        endPoint.b = ( block.y >>  2 ) & 0xFE;
        endPoint.a = ( block.y >> 16 ) & 0xFE;
        endPoint |= ( block.y >> 31 ) & 0x01;
    }
    else if ( mode == 7 )
    {
        endPoint.r = ( block.x >> 11 ) & 0xF8;
        endPoint.g = ( block.y <<  1 ) & 0xF8;
        endPoint.b = ( block.y >> 19 ) & 0xF8;
        endPoint.a = ( block.z >>  7 ) & 0xF8;
        endPoint |= ( ( block.z >> 28 ) & 0x04 ) | ( endPoint >> 6 );
    }
}
void extract_and_decode_endpoints_01( out uint4 endPoint, uint mode, uint4 block )
{
    if ( mode == 0 )
    {
        endPoint.r = ( block.x >>  5 ) & 0xF0;
        endPoint.g = ( block.y <<  3 ) & 0xF0;
        endPoint.b = ( block.y >> 21 ) & 0xF0;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 11 ) & 0x08 ) | ( endPoint.rgb >>  5 );
    }
    else if ( mode == 1 )
    {
        endPoint.r = ( block.x >> 12 ) & 0xFC;
        endPoint.g = ( block.y >>  4 ) & 0xFC;
        endPoint.b = ( ( block.y >> 28 ) & 0xFC ) | ( ( block.z <<  4 ) & 0xFC );
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 15 ) & 0x02 ) | ( endPoint.rgb >>  7 );
    }
    else if ( mode == 2 )
    {
        endPoint.r = ( block.x >> 11 ) & 0xF8;
        endPoint.g = ( block.y >>  9 ) & 0xF8;
        endPoint.b = ( block.z >>  7 ) & 0xF8;
        endPoint.a = 255;
        endPoint |= endPoint >>  5;
    }
    else if ( mode == 3 )
    {
        endPoint.r = ( block.x >> 16 ) & 0xFE;
        endPoint.g = ( block.y >> 12 ) & 0xFE;
        endPoint.b = ( block.z >>  8 ) & 0xFE;
        endPoint.a = 255;
        endPoint.rgb |= ( block.z >> 31 ) & 0x01;
    }
    else if ( mode == 4 )
    {
        endPoint.r = ( block.x >> 10 ) & 0xF8;
        endPoint.g = ( block.x >> 20 ) & 0xF8;
        endPoint.b = ( block.y <<  2 ) & 0xF8;
        endPoint.a = ( block.y >> 10 ) & 0xFC;
        endPoint.rgb |= endPoint.rgb >>  5;
        endPoint.a |= endPoint.a >>  6;
    }
    else if ( mode == 5 )
    {
        endPoint.r = ( block.x >> 14 ) & 0xFE;
        endPoint.g = ( ( block.x >> 28 ) & 0xFE ) | ( ( block.y <<  4 ) & 0xFE );
        endPoint.b = ( block.y >> 10 ) & 0xFE;
        endPoint.a = ( ( block.y >> 26 ) & 0xFF ) | ( ( block.z <<  6 ) & 0xFF );
        endPoint.rgb |= endPoint.rgb >>  7;
    }
    else if ( mode == 6 )
    {
        endPoint.r = ( block.x >> 13 ) & 0xFE;
        endPoint.g = ( ( block.x >> 27 ) & 0xFE ) | ( ( block.y << 5 ) & 0xFE );
        endPoint.b = ( block.y >>  9 ) & 0xFE;
        endPoint.a = ( block.y >> 23 ) & 0xFE;
        endPoint |= ( block.z >>  0 ) & 0x01;
    }
    else if ( mode == 7 )
    {
        endPoint.r = ( block.x >> 16 ) & 0xF8;
        endPoint.g = ( block.y >>  4 ) & 0xF8;
        endPoint.b = ( block.y >> 24 ) & 0xF8;
        endPoint.a = ( block.z >> 12 ) & 0xF8;
        endPoint |= ( ( block.z >> 29 ) & 0x04 ) | ( endPoint >> 6 );
    }
}
void extract_and_decode_endpoints_10( out uint4 endPoint, uint mode, uint4 block )
{
    if ( mode == 0 )
    {
        endPoint.r = ( block.x >>  9 ) & 0xF0;
        endPoint.g = ( block.y >>  1 ) & 0xF0;
        endPoint.b = ( ( block.y >> 25 ) & 0xF0 ) | ( ( block.z <<  7 ) & 0xF0 );
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 12 ) & 0x08 ) | ( endPoint.rgb >>  5 );
    }
    else if ( mode == 1 )
    {
        endPoint.r = ( block.x >> 18 ) & 0xFC;
        endPoint.g = ( block.y >> 10 ) & 0xFC;
        endPoint.b = ( block.z >>  2 ) & 0xFC;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 16 ) & 0x02 ) | ( endPoint.rgb >>  7 );
    }
    else if ( mode == 2 )
    {
        endPoint.r = ( block.x >> 16 ) & 0xF8;
        endPoint.g = ( block.y >> 14 ) & 0xF8;
        endPoint.b = ( block.z >> 12 ) & 0xF8;
        endPoint.a = 255;
        endPoint |= endPoint >>  5;
    }
    else if ( mode == 3 )
    {
        endPoint.r = ( block.x >> 23 ) & 0xFE;
        endPoint.g = ( block.y >> 19 ) & 0xFE;
        endPoint.b = ( block.z >> 15 ) & 0xFE;
        endPoint.a = 255;
        endPoint.rgb |= ( block.w >>  0 ) & 0x01;
    }
    else if ( mode == 4 )
    {
        endPoint = 0;
    }
    else if ( mode == 5 )
    {
        endPoint = 0;
    }
    else if ( mode == 6 )
    {
        endPoint = 0;
    }
    else if ( mode == 7 )
    {
        endPoint.r = ( block.x >> 21 ) & 0xF8;
        endPoint.g = ( block.y >>  9 ) & 0xF8;
        endPoint.b = ( block.z <<  3 ) & 0xF8;
        endPoint.a = ( block.z >> 17 ) & 0xF8;
        endPoint |= ( ( block.w <<  2 ) & 0x04 ) | ( endPoint >> 6 );
    }
}
void extract_and_decode_endpoints_11( out uint4 endPoint, uint mode, uint4 block )
{
    if ( mode == 0 )
    {
        endPoint.r = ( block.x >> 13 ) & 0xF0;
        endPoint.g = ( block.y >>  5 ) & 0xF0;
        endPoint.b = ( block.z <<  3 ) & 0xF0;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 13 ) & 0x08 ) | ( endPoint.rgb >>  5 );
    }
    else if ( mode == 1 )
    {
        endPoint.r = ( block.x >> 24 ) & 0xFC;
        endPoint.g = ( block.y >> 16 ) & 0xFC;
        endPoint.b = ( block.z >>  8 ) & 0xFC;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 16 ) & 0x02 ) | ( endPoint.rgb >>  7 );
    }
    else if ( mode == 2 )
    {
        endPoint.r = ( block.x >> 21 ) & 0xF8;
        endPoint.g = ( block.y >> 19 ) & 0xF8;
        endPoint.b = ( block.z >> 17 ) & 0xF8;
        endPoint.a = 255;
        endPoint |= endPoint >>  5;
    }
    else if ( mode == 3 )
    {
        endPoint.r = ( ( block.x >> 30 ) & 0xFE ) | ( ( block.y <<  2 ) & 0xFE );
        endPoint.g = ( ( block.y >> 26 ) & 0xFE ) | ( ( block.z << 6 ) & 0xFE );
        endPoint.b = ( block.z >> 22 ) & 0xFE;
        endPoint.a = 255;
        endPoint.rgb |= ( block.w >>  1 ) & 0x01;
    }
    else if ( mode == 4 )
    {
        endPoint = 0;
    }
    else if ( mode == 5 )
    {
        endPoint = 0;
    }
    else if ( mode == 6 )
    {
        endPoint = 0;
    }
    else if ( mode == 7 )
    {
        endPoint.r = ( ( block.x >> 26 ) & 0xF8 ) | ( ( block.y <<  6 ) & 0xF8 );
        endPoint.g = ( block.y >>  14 ) & 0xF8;
        endPoint.b = ( block.z >>  2 ) & 0xF8;
        endPoint.a = ( block.z >> 22 ) & 0xF8;
        endPoint |= ( ( block.w <<  1 ) & 0x04 ) | ( endPoint >> 6 );
    }
}
void extract_and_decode_endpoints_20( out uint4 endPoint, uint mode, uint4 block )
{
    if ( mode == 0 )
    {
        endPoint.r = ( block.x >> 17 ) & 0xF0;
        endPoint.g = ( block.y >>  9 ) & 0xF0;
        endPoint.b = ( block.z >>  1 ) & 0xF0;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 14 ) & 0x08 ) | ( endPoint.rgb >>  5 );
    }
    else if ( mode == 1 )
    {
        endPoint = 0;
    }
    else if ( mode == 2 )
    {
        endPoint.r = ( ( block.x >> 26 ) & 0xF8 ) | ( ( block.y <<  6 ) & 0xF8 );
        endPoint.g = ( block.y >> 24 ) & 0xF8;
        endPoint.b = ( block.z >> 22 ) & 0xF8;
        endPoint.a = 255;
        endPoint |= endPoint >>  5;
    }
    else if ( mode == 3 )
    {
        endPoint = 0;
    }
    else if ( mode == 4 )
    {
        endPoint = 0;
    }
    else if ( mode == 5 )
    {
        endPoint = 0;
    }
    else if ( mode == 6 )
    {
        endPoint = 0;
    }
    else if ( mode == 7 )
    {
        endPoint = 0;
    }
}
void extract_and_decode_endpoints_21( out uint4 endPoint, uint mode, uint4 block )
{
    if ( mode == 0 )
    {
        endPoint.r = ( block.x >> 21 ) & 0xF0;
        endPoint.g = ( block.y >> 13 ) & 0xF0;
        endPoint.b = ( block.z >>  5 ) & 0xF0;
        endPoint.a = 255;
        endPoint.rgb |= ( ( block.z >> 15 ) & 0x08 ) | ( endPoint.rgb >>  5 );
    }
    else if ( mode == 1 )
    {
        endPoint = 0;
    }
    else if ( mode == 2 )
    {
        endPoint.r = ( block.y <<  1 ) & 0xF8;
        endPoint.g = ( block.z <<  3 ) & 0xF8;
        endPoint.b = ( ( block.z >> 27 ) & 0xF8 ) | ( ( block.w <<  5 ) & 0xF8 );
        endPoint.a = 255;
        endPoint |= endPoint >>  5;
    }
    else if ( mode == 3 )
    {
        endPoint = 0;
    }
    else if ( mode == 4 )
    {
        endPoint = 0;
    }
    else if ( mode == 5 )
    {
        endPoint = 0;
    }
    else if ( mode == 6 )
    {
        endPoint = 0;
    }
    else if ( mode == 7 )
    {
        endPoint = 0;
    }
}

void extract_and_decode_endpoints( out uint2x4 endPoint[3], uint mode, uint4 block )
{
    if ( mode == 0 )
    {
        endPoint[0][0].r = ( block.x >>  1 ) & 0xF0; endPoint[0][0].g = ( ( block.x >> 25 ) & 0xF0 ) | ( ( block.y <<  7 ) & 0xF0 ); endPoint[0][0].b = ( block.y >> 17 ) & 0xF0; endPoint[0][0].a = 255;
        endPoint[0][1].r = ( block.x >>  5 ) & 0xF0; endPoint[0][1].g = ( block.y <<  3 ) & 0xF0; endPoint[0][1].b = ( block.y >> 21 ) & 0xF0; endPoint[0][1].a = 255;
        endPoint[1][0].r = ( block.x >>  9 ) & 0xF0; endPoint[1][0].g = ( block.y >>  1 ) & 0xF0; endPoint[1][0].b = ( ( block.y >> 25 ) & 0xF0 ) | ( ( block.z <<  7 ) & 0xF0 ); endPoint[1][0].a = 255;
        endPoint[1][1].r = ( block.x >> 13 ) & 0xF0; endPoint[1][1].g = ( block.y >>  5 ) & 0xF0; endPoint[1][1].b = ( block.z <<  3 ) & 0xF0; endPoint[1][1].a = 255;
        endPoint[2][0].r = ( block.x >> 17 ) & 0xF0; endPoint[2][0].g = ( block.y >>  9 ) & 0xF0; endPoint[2][0].b = ( block.z >>  1 ) & 0xF0; endPoint[2][0].a = 255;
        endPoint[2][1].r = ( block.x >> 21 ) & 0xF0; endPoint[2][1].g = ( block.y >> 13 ) & 0xF0; endPoint[2][1].b = ( block.z >>  5 ) & 0xF0; endPoint[2][1].a = 255;
        endPoint[0][0].rgb |= ( ( block.z >> 10 ) & 0x08 ) | ( endPoint[0][0].rgb >>  5 );
        endPoint[0][1].rgb |= ( ( block.z >> 11 ) & 0x08 ) | ( endPoint[0][1].rgb >>  5 );
        endPoint[1][0].rgb |= ( ( block.z >> 12 ) & 0x08 ) | ( endPoint[1][0].rgb >>  5 );
        endPoint[1][1].rgb |= ( ( block.z >> 13 ) & 0x08 ) | ( endPoint[1][1].rgb >>  5 );
        endPoint[2][0].rgb |= ( ( block.z >> 14 ) & 0x08 ) | ( endPoint[2][0].rgb >>  5 );
        endPoint[2][1].rgb |= ( ( block.z >> 15 ) & 0x08 ) | ( endPoint[2][1].rgb >>  5 );
    }
    else if ( mode == 1 )
    {
        endPoint[0][0].r = ( block.x >>  6 ) & 0xFC; endPoint[0][0].g = ( block.y <<  2 ) & 0xFC; endPoint[0][0].b = ( block.y >> 22 ) & 0xFC; endPoint[0][0].a = 255;
        endPoint[0][1].r = ( block.x >> 12 ) & 0xFC; endPoint[0][1].g = ( block.y >>  4 ) & 0xFC; endPoint[0][1].b = ( ( block.y >> 28 ) & 0xFC ) | ( ( block.z <<  4 ) & 0xFC ); endPoint[0][1].a = 255;
        endPoint[1][0].r = ( block.x >> 18 ) & 0xFC; endPoint[1][0].g = ( block.y >> 10 ) & 0xFC; endPoint[1][0].b = ( block.z >>  2 ) & 0xFC; endPoint[1][0].a = 255;
        endPoint[1][1].r = ( block.x >> 24 ) & 0xFC; endPoint[1][1].g = ( block.y >> 16 ) & 0xFC; endPoint[1][1].b = ( block.z >>  8 ) & 0xFC; endPoint[1][1].a = 255;
        endPoint[0][0].rgb |= ( ( block.z >> 15 ) & 0x02 ) | ( endPoint[0][0].rgb >>  7 );
        endPoint[0][1].rgb |= ( ( block.z >> 15 ) & 0x02 ) | ( endPoint[0][1].rgb >>  7 );
        endPoint[1][0].rgb |= ( ( block.z >> 16 ) & 0x02 ) | ( endPoint[1][0].rgb >>  7 );
        endPoint[1][1].rgb |= ( ( block.z >> 16 ) & 0x02 ) | ( endPoint[1][1].rgb >>  7 );
        endPoint[2] = 0;
    }
    else if ( mode == 2 )
    {
        endPoint[0][0].r = ( block.x >>  6 ) & 0xF8; endPoint[0][0].g = ( block.y >>  4 ) & 0xF8; endPoint[0][0].b = ( block.z >>  2 ) & 0xF8; endPoint[0][0].a = 255;
        endPoint[0][1].r = ( block.x >> 11 ) & 0xF8; endPoint[0][1].g = ( block.y >>  9 ) & 0xF8; endPoint[0][1].b = ( block.z >>  7 ) & 0xF8; endPoint[0][1].a = 255;
        endPoint[1][0].r = ( block.x >> 16 ) & 0xF8; endPoint[1][0].g = ( block.y >> 14 ) & 0xF8; endPoint[1][0].b = ( block.z >> 12 ) & 0xF8; endPoint[1][0].a = 255;
        endPoint[1][1].r = ( block.x >> 21 ) & 0xF8; endPoint[1][1].g = ( block.y >> 19 ) & 0xF8; endPoint[1][1].b = ( block.z >> 17 ) & 0xF8; endPoint[1][1].a = 255;
        endPoint[2][0].r = ( ( block.x >> 26 ) & 0xF8 ) | ( ( block.y <<  6 ) & 0xF8 ); endPoint[2][0].g = ( block.y >> 24 ) & 0xF8; endPoint[2][0].b = ( block.z >> 22 ) & 0xF8; endPoint[2][0].a = 255;
        endPoint[2][1].r = ( block.y <<  1 ) & 0xF8; endPoint[2][1].g = ( block.z <<  3 ) & 0xF8; endPoint[2][1].b = ( ( block.z >> 27 ) & 0xF8 ) | ( ( block.w <<  5 ) & 0xF8 ); endPoint[2][1].a = 255;
        endPoint[0] |= endPoint[0] >>  5;
        endPoint[1] |= endPoint[1] >>  5;
        endPoint[2] |= endPoint[2] >>  5;
    }
    else if ( mode == 3 )
    {
        endPoint[0][0].r = ( block.x >>  9 ) & 0xFE; endPoint[0][0].g = ( block.y >>  5 ) & 0xFE; endPoint[0][0].b = ( block.z >>  1 ) & 0xFE; endPoint[0][0].a = 255;
        endPoint[0][1].r = ( block.x >> 16 ) & 0xFE; endPoint[0][1].g = ( block.y >> 12 ) & 0xFE; endPoint[0][1].b = ( block.z >>  8 ) & 0xFE; endPoint[0][1].a = 255;
        endPoint[1][0].r = ( block.x >> 23 ) & 0xFE; endPoint[1][0].g = ( block.y >> 19 ) & 0xFE; endPoint[1][0].b = ( block.z >> 15 ) & 0xFE; endPoint[1][0].a = 255;
        endPoint[1][1].r = ( ( block.x >> 30 ) & 0xFE ) | ( ( block.y <<  2 ) & 0xFE ); endPoint[1][1].g = ( ( block.y >> 26 ) & 0xFE ) | ( ( block.z << 6 ) & 0xFE ); endPoint[1][1].b = ( block.z >> 22 ) & 0xFE; endPoint[1][1].a = 255;
        endPoint[0][0].rgb |= ( block.z >> 30 ) & 0x01;
        endPoint[0][1].rgb |= ( block.z >> 31 ) & 0x01;
        endPoint[1][0].rgb |= ( block.w >>  0 ) & 0x01;
        endPoint[1][1].rgb |= ( block.w >>  1 ) & 0x01;
        endPoint[2] = 0;
    }
    else if ( mode == 4 )
    {
        endPoint[0][0].r = ( block.x >>  5 ) & 0xF8; endPoint[0][0].g = ( block.x >> 15 ) & 0xF8; endPoint[0][0].b = ( ( block.x >> 25 ) & 0xF8 ) | ( ( block.y <<  7 ) & 0xF8 ); endPoint[0][0].a = ( block.y >>  4 ) & 0xFC;
        endPoint[0][1].r = ( block.x >> 10 ) & 0xF8; endPoint[0][1].g = ( block.x >> 20 ) & 0xF8; endPoint[0][1].b = ( block.y <<  2 ) & 0xF8;  endPoint[0][1].a = ( block.y >> 10 ) & 0xFC;
        endPoint[0][0].rgb |= endPoint[0][0].rgb >>  5; endPoint[0][0].a |= endPoint[0][0].a >>  6;
        endPoint[0][1].rgb |= endPoint[0][1].rgb >>  5; endPoint[0][1].a |= endPoint[0][1].a >>  6;
        endPoint[1] = 0;
        endPoint[2] = 0;
    }
    else if ( mode == 5 )
    {
        endPoint[0][0].r = ( block.x >>  7 ) & 0xFE; endPoint[0][0].g = ( block.x >> 21 ) & 0xFE; endPoint[0][0].b = ( block.y >>  3 ) & 0xFE; endPoint[0][0].a = ( block.y >> 18 ) & 0xFF;
        endPoint[0][1].r = ( block.x >> 14 ) & 0xFE; endPoint[0][1].g = ( ( block.x >> 28 ) & 0xFE ) | ( ( block.y <<  4 ) & 0xFE ); endPoint[0][1].b = ( block.y >> 10 ) & 0xFE; endPoint[0][1].a = ( ( block.y >> 26 ) & 0xFF ) | ( ( block.z <<  6 ) & 0xFF );
        endPoint[0][0].rgb |= endPoint[0][0].rgb >>  7;
        endPoint[0][1].rgb |= endPoint[0][1].rgb >>  7;
        endPoint[1] = 0;
        endPoint[2] = 0;
    }
    else if ( mode == 6 )
    {
        endPoint[0][0].r = ( block.x >>  6 ) & 0xFE; endPoint[0][0].g = ( block.x >> 20 ) & 0xFE; endPoint[0][0].b = ( block.y >>  2 ) & 0xFE; endPoint[0][0].a = ( block.y >> 16 ) & 0xFE;
        endPoint[0][1].r = ( block.x >> 13 ) & 0xFE; endPoint[0][1].g = ( ( block.x >> 27 ) & 0xFE ) | ( ( block.y << 5 ) & 0xFE );  endPoint[0][1].b = ( block.y >>  9 ) & 0xFE; endPoint[0][1].a = ( block.y >> 23 ) & 0xFE;
        endPoint[0][0] |= ( block.y >> 31 ) & 0x01;
        endPoint[0][1] |= ( block.z >>  0 ) & 0x01;
        endPoint[1] = 0;
        endPoint[2] = 0;
    }
    else if ( mode == 7 )
    {
        endPoint[0][0].r = ( block.x >> 11 ) & 0xF8; endPoint[0][0].g = ( block.y <<  1 ) & 0xF8; endPoint[0][0].b = ( block.y >> 19 ) & 0xF8; endPoint[0][0].a = ( block.z >>  7 ) & 0xF8;
        endPoint[0][1].r = ( block.x >> 16 ) & 0xF8; endPoint[0][1].g = ( block.y >>  4 ) & 0xF8; endPoint[0][1].b = ( block.y >> 24 ) & 0xF8; endPoint[0][1].a = ( block.z >> 12 ) & 0xF8;
        endPoint[1][0].r = ( block.x >> 21 ) & 0xF8; endPoint[1][0].g = ( block.y >>  9 ) & 0xF8; endPoint[1][0].b = ( block.z <<  3 ) & 0xF8; endPoint[1][0].a = ( block.z >> 17 ) & 0xF8;
        endPoint[1][1].r = ( ( block.x >> 26 ) & 0xF8 ) | ( ( block.y <<  6 ) & 0xF8 ); endPoint[1][1].g = ( block.y >>  14 ) & 0xF8; endPoint[1][1].b = ( block.z >>  2 ) & 0xF8; endPoint[1][1].a = ( block.z >> 22 ) & 0xF8;
        endPoint[0][0] |= ( ( block.z >> 28 ) & 0x04 ) | ( endPoint[0][0] >> 6 );
        endPoint[0][1] |= ( ( block.z >> 29 ) & 0x04 ) | ( endPoint[0][1] >> 6 );
        endPoint[1][0] |= ( ( block.w <<  2 ) & 0x04 ) | ( endPoint[1][0] >> 6 );
        endPoint[1][1] |= ( ( block.w <<  1 ) & 0x04 ) | ( endPoint[1][1] >> 6 );
        endPoint[2] = 0;
    }
}
void get_index( out uint alpha_index, out uint color_index, out uint subset_index, uint x, uint y, uint mode, uint partition, uint4 block )
{
    uint i = y * 4 + x;
    if ( mode == 0 ) //64 <= partition < 64 + 16
    {
        if ( i == 0 )
            color_index = ( block.z >> 19 ) & 0x03;
        else if ( i <  candidateFixUpIndex1DOrdered[partition][0] )
        {
            if ( i < 4 )
                color_index = ( block.z >> ( i * 3 + 18 ) ) & 0x07;
            else if ( i == 4 )
                color_index = ( ( block.z >> ( i * 3 + 18 ) ) & 0x03 ) | ( ( block.w << 2 ) & 0x04 );
            else
                color_index = ( block.w >> ( i * 3 - 14 ) ) & 0x07;
        }
        else if ( i == candidateFixUpIndex1DOrdered[partition][0] )
        {
            if ( i <= 4 )
                color_index = ( block.z >> ( i * 3 + 18 ) ) & 0x03;
            else
                color_index = ( block.w >> ( i * 3 - 14 ) ) & 0x03;
        }
        else if ( i <  candidateFixUpIndex1DOrdered[partition][1] )
        {
            if ( i <= 4 )
                color_index = ( block.z >> ( i * 3 + 17 ) ) & 0x07;
            else
                color_index = ( block.w >> ( i * 3 - 15 ) ) & 0x07;
        }
        else if ( i == candidateFixUpIndex1DOrdered[partition][1] ) //i >= 8
            color_index = ( block.w >> ( i * 3 - 15 ) ) & 0x03;
        else //i >= 9
            color_index = ( block.w >> ( i * 3 - 16 ) ) & 0x07;

        alpha_index = 0; //Not used
        subset_index = ( candidateSectionCompressed[partition] >> ( 30 - i * 2 ) ) & 0x03;
    }
    else if ( mode == 1 )
    {
        if ( i == 0 )
            color_index = ( block.z >> 18 ) & 0x03;
        else if ( i <  candidateFixUpIndex1DOrdered[partition][0] )
        {
            if ( i < 5 )
                color_index = ( block.z >> ( i * 3 + 17 ) ) & 0x07;
            else
                color_index = ( block.w >> ( i * 3 - 15 ) ) & 0x07;
        }
        else if ( i == candidateFixUpIndex1DOrdered[partition][0] ) //i can't be 5
        {
            if ( i < 5 )
                color_index = ( block.z >> ( i * 3 + 17 ) ) & 0x03;
            else
                color_index = ( block.w >> ( i * 3 - 15 ) ) & 0x03;
        }
        else
        {
            if ( i < 5 )
                color_index = ( block.z >> ( i * 3 + 16 ) ) & 0x07;
            else if ( i == 5 )
                color_index = ( ( block.z >> ( i * 3 + 16 ) ) & 0x01 ) | ( ( block.w << 1 ) & 0x06 );
            else
                color_index = ( block.w >> ( i * 3 - 16 ) ) & 0x07;
        }
        
        alpha_index = 0; //Not used
        subset_index = ( candidateSectionCompressed[partition] >> ( 30 - i * 2 ) ) & 0x03;
    }
    else if ( mode == 2 )
    {
        if ( i == 0 )
            color_index = ( block.w >> 3 ) & 0x01;
        else if ( i <  candidateFixUpIndex1DOrdered[partition][0] )
            color_index = ( block.w >> ( i * 2 + 2 ) ) & 0x03;
        else if ( i == candidateFixUpIndex1DOrdered[partition][0] )
            color_index = ( block.w >> ( i * 2 + 2 ) ) & 0x01;
        else if ( i <  candidateFixUpIndex1DOrdered[partition][1] )
            color_index = ( block.w >> ( i * 2 + 1 ) ) & 0x03;
        else if ( i == candidateFixUpIndex1DOrdered[partition][1] )
            color_index = ( block.w >> ( i * 2 + 1 ) ) & 0x01;
        else
            color_index = ( block.w >> ( i * 2 ) ) & 0x03;

        alpha_index = 0; //Not used
        subset_index = ( candidateSectionCompressed[partition] >> ( 30 - i * 2 ) ) & 0x03;
    }
    else if ( mode == 3 )
    {
        if ( i == 0 )
            color_index = ( block.w >> 2 ) & 0x01;
        else if ( i <  candidateFixUpIndex1DOrdered[partition][0] )
            color_index = ( block.w >> ( i * 2 + 1 ) ) & 0x03;
        else if ( i == candidateFixUpIndex1DOrdered[partition][0] )
            color_index = ( block.w >> ( i * 2 + 1 ) ) & 0x01;
        else
            color_index = ( block.w >> ( i * 2 ) ) & 0x03;

        alpha_index = 0; //Not used
        subset_index = ( candidateSectionCompressed[partition] >> ( 30 - i * 2 ) ) & 0x03;
    }
    else if ( mode == 4 )
    {
        if ( i == 0 )
            color_index = ( block.y >> 18 ) & 0x01;
        else if ( i <  7 )
            color_index = ( block.y >> ( i * 2 + 17 ) ) & 0x03;
        else if ( i == 7 )
            color_index = ( ( block.y >> ( i * 2 + 17 ) ) & 0x01 ) | ( ( block.z << 1 ) & 0x02 );
        else
            color_index = ( block.z >> ( i * 2 - 15 ) ) & 0x03;
        
        if ( i == 0 )
            alpha_index = ( block.z >> 17 ) & 0x03;
        else if ( i <  5 )
            alpha_index = ( block.z >> ( i * 3 + 16 ) ) & 0x07;
        else if ( i == 5 )
            alpha_index = ( ( block.z >> ( i * 3 + 16 ) ) & 0x01 ) | ( ( block.w << 1 ) & 0x06 );
        else
            alpha_index = ( block.w >> ( i * 3 - 16 ) ) & 0x07;
        
        if ( block.x & 0x80 )
        {
            uint tmp = color_index;
            color_index = alpha_index;
            alpha_index = tmp;
        }
        
        subset_index = 0; //Not used
    }
    else if ( mode == 5 )
    {
        if ( i == 0 )
            color_index = ( block.z >> 2 ) & 0x01;
        else if ( i < 15 )
            color_index = ( block.z >> ( i * 2 + 1 ) ) & 0x03;
        else
            color_index = ( ( block.z >> 31 ) & 0x01 ) | ( ( block.w << 1 ) & 0x02 );
        
        if ( i == 0 )
            alpha_index = ( block.w >> 1 ) & 0x01;
        else
            alpha_index = ( block.w >> ( i * 2 ) ) & 0x03;
        
        subset_index = 0; //Not used
    }
    else if ( mode == 6 )
    {
        if ( i == 0 )
            color_index = ( block.z >> 1 ) & 0x07;
        else if ( i < 8 )
            color_index = ( block.z >> ( i * 4 ) ) & 0x0F;
        else
            color_index = ( block.w >> ( i * 4 - 32 ) ) & 0x0F;
        
        alpha_index = color_index;
        subset_index = 0; //Not used
    }
    else // mode == 7
    {
        if ( i == 0 )
            color_index = ( block.w >> 2 ) & 0x01;
        else if ( i <  candidateFixUpIndex1DOrdered[partition][0] )
            color_index = ( block.w >> ( i * 2 + 1 ) ) & 0x03;
        else if ( i == candidateFixUpIndex1DOrdered[partition][0] )
            color_index = ( block.w >> ( i * 2 + 1 ) ) & 0x01;
        else
            color_index = ( block.w >> ( i * 2 ) ) & 0x03;
            
        alpha_index = color_index;
        subset_index = ( candidateSectionCompressed[partition] >> ( 30 - i * 2 ) ) & 0x03;
    }
}
static const uint aWeight2[4] = {0, 21, 43, 64};
static const uint aWeight3[8] = {0, 9, 18, 27, 37, 46, 55, 64};
static const uint aWeight4[16] = {0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};
uint3 interpolate_color( uint color_index, uint index_prec, uint2x4 endPoint )
{
    if ( index_prec == 2 )
        return ( ( 64 - aWeight2[color_index] ) * endPoint[0].rgb + aWeight2[color_index] * endPoint[1].rgb + 32 ) >> 6;
    if ( index_prec == 3 )
        return ( ( 64 - aWeight3[color_index] ) * endPoint[0].rgb + aWeight3[color_index] * endPoint[1].rgb + 32 ) >> 6;
    return ( ( 64 - aWeight4[color_index] ) * endPoint[0].rgb + aWeight4[color_index] * endPoint[1].rgb + 32 ) >> 6;
}
uint interpolate_alpha( uint alpha_index, uint index_prec, uint2x4 endPoint )
{
    if ( index_prec == 2 )
        return ( ( 64 - aWeight2[alpha_index] ) * endPoint[0].a + aWeight2[alpha_index] * endPoint[1].a + 32 ) >> 6;
    if ( index_prec == 3 )
        return ( ( 64 - aWeight3[alpha_index] ) * endPoint[0].a + aWeight3[alpha_index] * endPoint[1].a + 32 ) >> 6;
    return ( ( 64 - aWeight4[alpha_index] ) * endPoint[0].a + aWeight4[alpha_index] * endPoint[1].a + 32 ) >> 6;
}