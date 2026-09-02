// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: groupId
// CHECK: bufferLoad
// CHECK: textureLoad
// CHECK: UMax
// CHECK: UMin
// CHECK: barrier

// CHECK: IMad
// CHECK: barrier

// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: BC7Encode.hlsl
//
// The Compute Shader for BC7 Encoder
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#define REF_DEVICE

#define CHAR_LENGTH			8
#define NCHANNELS			4
#define	BC7_UNORM			98
#define MAX_UINT			0xFFFFFFFF
#define MIN_UINT			0

static const uint candidateSectionBit[64] = //Associated to partition 0-63
{
    0xCCCC, 0x8888, 0xEEEE, 0xECC8,
    0xC880, 0xFEEC, 0xFEC8, 0xEC80,
    0xC800, 0xFFEC, 0xFE80, 0xE800,
    0xFFE8, 0xFF00, 0xFFF0, 0xF000,
    0xF710, 0x008E, 0x7100, 0x08CE,
    0x008C, 0x7310, 0x3100, 0x8CCE,
    0x088C, 0x3110, 0x6666, 0x366C,
    0x17E8, 0x0FF0, 0x718E, 0x399C,
    0xaaaa, 0xf0f0, 0x5a5a, 0x33cc, 
    0x3c3c, 0x55aa, 0x9696, 0xa55a, 
    0x73ce, 0x13c8, 0x324c, 0x3bdc, 
    0x6996, 0xc33c, 0x9966, 0x660, 
    0x272, 0x4e4, 0x4e40, 0x2720, 
    0xc936, 0x936c, 0x39c6, 0x639c, 
    0x9336, 0x9cc6, 0x817e, 0xe718, 
    0xccf0, 0xfcc, 0x7744, 0xee22, 
};
static const uint candidateSectionBit2[64] = //Associated to partition 64-127
{
    0xf60008cc, 0x73008cc8, 0x3310cc80, 0xceec00, 
    0xcc003300, 0xcc0000cc, 0xccff00, 0x3300cccc, 
    0xf0000f00, 0xf0000ff0, 0xff0000f0, 0x88884444, 
    0x88886666, 0xcccc2222, 0xec80136c, 0x7310008c, 
    0xc80036c8, 0x310008ce, 0xccc03330, 0xcccf000, 
    0xee0000ee, 0x77008888, 0xcc0022c0, 0x33004430, 
    0xcc0c22, 0xfc880344, 0x6606996, 0x66009960, 
    0xc88c0330, 0xf9000066, 0xcc0c22c, 0x73108c00, 

    0xec801300, 0x8cec400, 0xec80004c, 0x44442222, 
    0xf0000f0, 0x49242492, 0x42942942, 0xc30c30c, 
    0x3c0c03c, 0xff0000aa, 0x5500aa00, 0xcccc3030, 
    0xc0cc0c0, 0x66669090, 0xff0a00a, 0x5550aaa0, 
    0xf0000aaa, 0xe0ee0e0, 0x88887070, 0x99906660, 
    0xe00e0ee0, 0x88880770, 0xf0000666, 0x99006600, 
    0xff000066, 0xc00c0cc0, 0xcccc0330, 0x90006000, 
    0x8088080, 0xeeee1010, 0xfff0000a, 0x731008ce, 
};
static const uint2 candidateFixUpIndex1D[128] = 
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
};
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
    { 3,15},{ 6,15},{ 6,15},{ 8,15}, //The Spec doesn't mark the first fixed up index in this row, so I apply 15 for them, and seems correct
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

static const uint aWeight[3][16] = { {0,  4,  9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64},
                                    {0,  9, 18, 27, 37, 46, 55, 64,  0,  0,  0,  0,  0,  0,  0,  0},
                                    {0, 21, 43, 64,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0} };
                                //0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64
static const uint aStep[3][64] = {  { 0, 0, 0, 1, 1, 1, 1, 2,
                                    2, 2, 2, 2, 3, 3, 3, 3,
                                    4, 4, 4, 4, 5, 5, 5, 5,
                                    6, 6, 6, 6, 6, 7, 7, 7,
                                    7, 8, 8, 8, 8, 9, 9, 9,
                                    9,10,10,10,10,10,11,11,
                                   11,11,12,12,12,12,13,13,
                                   13,13,14,14,14,14,15,15 },
                                //0, 9, 18, 27, 37, 46, 55, 64
                                    { 0,0,0,0,0,1,1,1,
                                    1,1,1,1,1,1,2,2,
                                    2,2,2,2,2,2,2,3,
                                    3,3,3,3,3,3,3,3,
                                    3,4,4,4,4,4,4,4,
                                    4,4,5,5,5,5,5,5,
                                    5,5,5,6,6,6,6,6,
                                    6,6,6,6,7,7,7,7 },
                                //0, 21, 43, 64
                                    { 0,0,0,0,0,0,0,0,
                                    0,0,0,1,1,1,1,1,
                                    1,1,1,1,1,1,1,1,
                                    1,1,1,1,1,1,1,1,
                                    1,2,2,2,2,2,2,2,
                                    2,2,2,2,2,2,2,2,
                                    2,2,2,2,2,2,3,3,
                                    3,3,3,3,3,3,3,3 } };

cbuffer cbCS : register( b0 )
{
    uint g_tex_width;
    uint g_num_block_x;
    uint g_format;
    uint g_mode_id;
    uint g_start_block_id;
    uint g_num_total_blocks;
};

//Forward declaration
void compress_endpoints0( inout uint2x4 endPoint ); //Mode = 0
void compress_endpoints1( inout uint2x4 endPoint ); //Mode = 1
void compress_endpoints2( inout uint2x4 endPoint ); //Mode = 2
void compress_endpoints3( inout uint2x4 endPoint ); //Mode = 3
void compress_endpoints7( inout uint2x4 endPoint ); //Mode = 7
void compress_endpoints6( inout uint2x4 endPoint ); //Mode = 6
void compress_endpoints4( inout uint2x4 endPoint ); //Mode = 4
void compress_endpoints5( inout uint2x4 endPoint ); //Mode = 5

void block_package0( out uint4 block, uint partition, uint threadBase ); //Mode0
void block_package1( out uint4 block, uint partition, uint threadBase ); //Mode1
void block_package2( out uint4 block, uint partition, uint threadBase ); //Mode2
void block_package3( out uint4 block, uint partition, uint threadBase ); //Mode3
void block_package4( out uint4 block, uint rotation, uint index_selector, uint threadBase ); //Mode4
void block_package5( out uint4 block, uint rotation, uint threadBase ); //Mode5
void block_package6( out uint4 block, uint threadBase ); //Mode6
void block_package7( out uint4 block, uint partition, uint threadBase ); //Mode7


void swap(inout uint4 lhs, inout uint4 rhs)
{
    int4 tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}
void swap(inout uint3 lhs, inout uint3 rhs)
{
    int3 tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}
void swap(inout uint lhs, inout uint rhs)
{
    int tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}


Texture2D g_Input : register( t0 ); 
StructuredBuffer<uint4> g_InBuff : register( t1 );

RWStructuredBuffer<uint4> g_OutBuff : register( u0 );

#define THREAD_GROUP_SIZE	64
#define BLOCK_SIZE_Y		4
#define BLOCK_SIZE_X		4
#define BLOCK_SIZE			(BLOCK_SIZE_Y * BLOCK_SIZE_X)

struct BufferShared
{
    uint4 pixel;
    uint error;
    uint mode;
    uint partition;
    uint index_selector;
    uint rotation;
    uint4 endPoint_low;
    uint4 endPoint_high;
};
groupshared BufferShared shared_temp[THREAD_GROUP_SIZE];

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void main( uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID )
{
    const uint MAX_USED_THREAD = 16;
    uint BLOCK_IN_GROUP = THREAD_GROUP_SIZE / MAX_USED_THREAD;
    uint blockInGroup = GI / MAX_USED_THREAD;
    uint blockID = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup;
    uint threadBase = blockInGroup * MAX_USED_THREAD;
    uint threadInBlock = GI - threadBase;

#ifndef REF_DEVICE
    if (blockID >= g_num_total_blocks)
    {
        return;
    }
#endif

    uint block_y = blockID / g_num_block_x;
    uint block_x = blockID - block_y * g_num_block_x;
    uint base_x = block_x * BLOCK_SIZE_X;
    uint base_y = block_y * BLOCK_SIZE_Y;
    
    if (threadInBlock < 16)
    {
        shared_temp[GI].pixel = clamp(uint4(g_Input.Load( uint3( base_x + threadInBlock % 4, base_y + threadInBlock / 4, 0 ) ) * 255), 0, 255);

        shared_temp[GI].endPoint_low = shared_temp[GI].pixel;
        shared_temp[GI].endPoint_high = shared_temp[GI].pixel;
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (threadInBlock < 8)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 8].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 8].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 4)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 4].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 4].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 2)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 2].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 2].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 1)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 1].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 1].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    uint2x4 endPoint;
    endPoint[0] = shared_temp[threadBase].endPoint_low;
    endPoint[1] = shared_temp[threadBase].endPoint_high;

    uint error = 0xFFFFFFFF;
    uint mode = 0;
    uint index_selector = 0;
    uint rotation = 0;

    uint2 indexPrec;
    if (threadInBlock < 8)
    {
        if (0 == (threadInBlock & 1))
        {
            //2 represents 2bit index precision; 1 represents 3bit index precision
            indexPrec = uint2( 2, 1 );
        }
        else
        {
            //2 represents 2bit index precision; 1 represents 3bit index precision
            index_selector = 1;
            indexPrec = uint2( 1, 2 );
        }
    }
    else
    {
         //2 represents 2bit index precision
        indexPrec = uint2( 2, 2 );
    }

    uint4 pixel_r;
    uint color_index;
    uint alpha_index;
    int4 span;
    int2 span_norm_sqr;
    int2 dotProduct;
    if (threadInBlock < 12)
    {
        if ((threadInBlock < 2) || (8 == threadInBlock)) // rotation = 0
        {
            rotation = 0;
        }
        else if ((threadInBlock < 4) || (9 == threadInBlock)) // rotation = 1
        {
            endPoint[0].ra = endPoint[0].ar;
            endPoint[1].ra = endPoint[1].ar;

            rotation = 1;
        }
        else if ((threadInBlock < 6) || (10 == threadInBlock)) // rotation = 2
        {
            endPoint[0].ga = endPoint[0].ag;
            endPoint[1].ga = endPoint[1].ag;

            rotation = 2;
        }
        else if ((threadInBlock < 8) || (11 == threadInBlock)) // rotation = 3
        {
            endPoint[0].ba = endPoint[0].ab;
            endPoint[1].ba = endPoint[1].ab;

            rotation = 3;
        }

        if (threadInBlock < 8)
        {
            mode = 4;
            compress_endpoints4( endPoint );
        }
        else
        {
            mode = 5;
            compress_endpoints5( endPoint );
        }

        uint4 pixel = shared_temp[threadBase + 0].pixel;
        if (1 == rotation)
        {
            pixel.ra = pixel.ar;
        }
        else if (2 == rotation)
        {
            pixel.ga = pixel.ag;
        }
        else if (3 == rotation)
        {
            pixel.ba = pixel.ab;
        }

        span = endPoint[1] - endPoint[0];
        span_norm_sqr = uint2( dot( span.rgb, span.rgb ), span.a * span.a );
        dotProduct = int2( dot( span.rgb, pixel.rgb - endPoint[0].rgb ), span.a * ( pixel.a - endPoint[0].a ) );
        if ( span_norm_sqr.x > 0 && dotProduct.x > 0 && uint( dotProduct.x * 63.49999 ) > uint( 32 * span_norm_sqr.x ) )
        {
            span.rgb = -span.rgb;
            swap(endPoint[0].rgb, endPoint[1].rgb);
        }
        if ( span_norm_sqr.y > 0 && dotProduct.y > 0 && uint( dotProduct.y * 63.49999 ) > uint( 32 * span_norm_sqr.y ) )
        {
            span.a = -span.a;
            swap(endPoint[0].a, endPoint[1].a);
        }

        error = 0;
        for ( uint i = 0; i < 16; i ++ )
        {
            pixel = shared_temp[threadBase + i].pixel;
            if (1 == rotation)
            {
                pixel.ra = pixel.ar;
            }
            else if (2 == rotation)
            {
                pixel.ga = pixel.ag;
            }
            else if (3 == rotation)
            {
                pixel.ba = pixel.ab;
            }

            dotProduct.x = dot( span.rgb, pixel.rgb - endPoint[0].rgb );
            color_index = ( span_norm_sqr.x <= 0 || dotProduct.x <= 0 ) ? 0
                : ( ( dotProduct.x < span_norm_sqr.x ) ? aStep[indexPrec.x][ uint( dotProduct.x * 63.49999 / span_norm_sqr.x ) ] : aStep[indexPrec.x][63] );
            dotProduct.y = dot( span.a, pixel.a - endPoint[0].a );
            alpha_index = ( span_norm_sqr.y <= 0 || dotProduct.y <= 0 ) ? 0
                : ( ( dotProduct.y < span_norm_sqr.y ) ? aStep[indexPrec.y][ uint( dotProduct.y * 63.49999 / span_norm_sqr.y ) ] : aStep[indexPrec.y][63] );

            if (index_selector)
            {
                swap(color_index, alpha_index);
            }

            pixel_r.rgb = ( ( 64 - aWeight[indexPrec.x][color_index] ) * endPoint[0].rgb
                 + aWeight[indexPrec.x][color_index] * endPoint[1] + 32 ) >> 6;
            pixel_r.a = ( ( 64 - aWeight[indexPrec.y][alpha_index] ) * endPoint[0].a
                 + aWeight[indexPrec.y][alpha_index] * endPoint[1] + 32 ) >> 6;

            pixel_r -= pixel;
            error += dot(pixel_r, pixel_r);
        }
    }
    else if (12 == threadInBlock)//Mode6
    {
        compress_endpoints6( endPoint );

        uint4 pixel = shared_temp[threadBase + 0].pixel;

        span = endPoint[1] - endPoint[0];
        span_norm_sqr = dot( span, span );
        dotProduct = dot( span, pixel - endPoint[0] );
        if ( span_norm_sqr.x > 0 && dotProduct.x >= 0 && uint( dotProduct.x * 63.49999 ) > uint( 32 * span_norm_sqr.x ) )
        {
            span = -span;
            swap(endPoint[0], endPoint[1]);
        }

        error = 0;
        for ( uint i = 0; i < 16; i ++ )
        {
            pixel = shared_temp[threadBase + i].pixel;

            dotProduct.x = dot( span, pixel - endPoint[0] );
            color_index = ( span_norm_sqr.x <= 0 || dotProduct.x <= 0 ) ? 0
                : ( ( dotProduct.x < span_norm_sqr.x ) ? aStep[0][ uint( dotProduct.x * 63.49999 / span_norm_sqr.x ) ] : aStep[0][63] );

            pixel_r = ( ( 64 - aWeight[0][color_index] ) * endPoint[0]
                + aWeight[0][color_index] * endPoint[1] + 32 ) >> 6;
        
            pixel_r -= pixel;
            error += dot(pixel_r, pixel_r);
        }

        mode = 6;
        rotation = 0;
    }

    shared_temp[GI].error = error;
    shared_temp[GI].mode = mode;
    shared_temp[GI].index_selector = index_selector;
    shared_temp[GI].rotation = rotation;

#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (threadInBlock < 8)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 8].error )
        {
            shared_temp[GI].error = shared_temp[GI + 8].error;
            shared_temp[GI].mode = shared_temp[GI + 8].mode;
            shared_temp[GI].index_selector = shared_temp[GI + 8].index_selector;
            shared_temp[GI].rotation = shared_temp[GI + 8].rotation;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 4)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 4].error )
        {
            shared_temp[GI].error = shared_temp[GI + 4].error;
            shared_temp[GI].mode = shared_temp[GI + 4].mode;
            shared_temp[GI].index_selector = shared_temp[GI + 4].index_selector;
            shared_temp[GI].rotation = shared_temp[GI + 4].rotation;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 2)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 2].error )
        {
            shared_temp[GI].error = shared_temp[GI + 2].error;
            shared_temp[GI].mode = shared_temp[GI + 2].mode;
            shared_temp[GI].index_selector = shared_temp[GI + 2].index_selector;
            shared_temp[GI].rotation = shared_temp[GI + 2].rotation;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 1)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 1].error )
        {
            shared_temp[GI].error = shared_temp[GI + 1].error;
            shared_temp[GI].mode = shared_temp[GI + 1].mode;
            shared_temp[GI].index_selector = shared_temp[GI + 1].index_selector;
            shared_temp[GI].rotation = shared_temp[GI + 1].rotation;
        }

        g_OutBuff[blockID] = uint4(shared_temp[GI].error, (shared_temp[GI].index_selector << 31) | shared_temp[GI].mode,
            0, shared_temp[GI].rotation);
    }
}

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void TryMode137CS( uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID )
{
    const uint MAX_USED_THREAD = 64;
    uint BLOCK_IN_GROUP = THREAD_GROUP_SIZE / MAX_USED_THREAD;
    uint blockInGroup = GI / MAX_USED_THREAD;
    uint blockID = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup;
    uint threadBase = blockInGroup * MAX_USED_THREAD;
    uint threadInBlock = GI - threadBase;

    uint block_y = blockID / g_num_block_x;
    uint block_x = blockID - block_y * g_num_block_x;
    uint base_x = block_x * BLOCK_SIZE_X;
    uint base_y = block_y * BLOCK_SIZE_Y;
    
    if (threadInBlock < 16)
    {
        shared_temp[GI].pixel = clamp(uint4(g_Input.Load( uint3( base_x + threadInBlock % 4, base_y + threadInBlock / 4, 0 ) ) * 255), 0, 255);
    }
    GroupMemoryBarrierWithGroupSync();

    shared_temp[GI].error = 0xFFFFFFFF;

    uint4 pixel_r;
    uint2x4 endPoint[2];
    uint color_index;
    if (threadInBlock < 64)
    {
        uint partition = threadInBlock;
        uint error = 0;

        endPoint[0][0] = MAX_UINT;
        endPoint[0][1] = MIN_UINT;
        endPoint[1][0] = MAX_UINT;
        endPoint[1][1] = MIN_UINT;
        uint bits = candidateSectionBit[partition];
        for ( uint i = 0; i < 16; i ++ )
        {
            uint4 pixel = shared_temp[threadBase + i].pixel;
            if ( (( bits >> i ) & 0x01) == 1 )
            {
                endPoint[1][0] = min( endPoint[1][0], pixel );
                endPoint[1][1] = max( endPoint[1][1], pixel );
            }
            else
            {
                endPoint[0][0] = min( endPoint[0][0], pixel );
                endPoint[0][1] = max( endPoint[0][1], pixel );
            }
        }

        for ( uint i = 0; i < 2; i ++ )
        {
            if (g_mode_id == 1)
            {
                compress_endpoints1( endPoint[i] );
            }
            else if (g_mode_id == 3)
            {
                compress_endpoints3( endPoint[i] );
            }
            else //if (g_mode_id == 7)
            {
                compress_endpoints7( endPoint[i] );
            }
        }

        int4 span[2];
        span[0] = endPoint[0][1] - endPoint[0][0];
        span[1] = endPoint[1][1] - endPoint[1][0];

        if (g_mode_id != 7)
        {
            span[0].w = span[1].w = 0;
        }

        int span_norm_sqr[2];
        span_norm_sqr[0] = dot( span[0], span[0] );
        span_norm_sqr[1] = dot( span[1], span[1] );

        int dotProduct = dot( span[0], shared_temp[threadBase + 0].pixel - endPoint[0][0] );
        if ( span_norm_sqr[0] > 0 && dotProduct > 0 && uint( dotProduct * 63.49999 ) > uint( 32 * span_norm_sqr[0] ) )
        {
            span[0] = -span[0];
            swap(endPoint[0][0], endPoint[0][1]);
        }
        dotProduct = dot( span[1], shared_temp[threadBase + candidateFixUpIndex1D[partition].x].pixel - endPoint[1][0] );
        if ( span_norm_sqr[1] > 0 && dotProduct > 0 && uint( dotProduct * 63.49999 ) > uint( 32 * span_norm_sqr[1] ) )
        {
            span[1] = -span[1];
            swap(endPoint[1][0], endPoint[1][1]);
        }

        uint step_selector;
        if (g_mode_id != 1)
        {
            step_selector = 2;
        }
        else
        {
            step_selector = 1;
        }

        uint bits2 = candidateSectionBit2[partition];
        for ( uint i = 0; i < 16; i ++ )
        {
            if (((bits >> i) & 0x01) == 1)
            {
                dotProduct = dot( span[1], shared_temp[threadBase + i].pixel - endPoint[1][0] );
                color_index = (span_norm_sqr[1] <= 0 || dotProduct <= 0) ? 0
                    : ((dotProduct < span_norm_sqr[1]) ? aStep[step_selector][uint(dotProduct * 63.49999 / span_norm_sqr[1])] : aStep[step_selector][63]);
            }
            else
            {
                dotProduct = dot( span[0], shared_temp[threadBase + i].pixel - endPoint[0][0] );
                color_index = (span_norm_sqr[0] <= 0 || dotProduct <= 0) ? 0
                    : ((dotProduct < span_norm_sqr[0]) ? aStep[step_selector][uint(dotProduct * 63.49999 / span_norm_sqr[0])] : aStep[step_selector][63]);
            }

            uint subset_index;
            if (g_mode_id == 7)
            {
                subset_index = (bits >> i) & 0x01;
            }
            else
            {
                subset_index = ((bits2 >> (i + 15)) & 0x02) | ((bits2 >> i) & 0x01);
            }

            pixel_r = ((64 - aWeight[step_selector][color_index]) * endPoint[subset_index][0]
                + aWeight[step_selector][color_index] * endPoint[subset_index][1] + 32) >> 6;

            pixel_r -= shared_temp[threadBase + i].pixel;
            if (g_mode_id != 7)
            {
                pixel_r.a = 0;
            }
            error += dot(pixel_r, pixel_r);
        }

        shared_temp[GI].error = error;
        shared_temp[GI].mode = g_mode_id;
        shared_temp[GI].partition = partition;
    }
    GroupMemoryBarrierWithGroupSync();

    if (threadInBlock < 32)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 32].error )
        {
            shared_temp[GI].error = shared_temp[GI + 32].error;
            shared_temp[GI].mode = shared_temp[GI + 32].mode;
            shared_temp[GI].partition = shared_temp[GI + 32].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
if (threadInBlock < 16)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 16].error )
        {
            shared_temp[GI].error = shared_temp[GI + 16].error;
            shared_temp[GI].mode = shared_temp[GI + 16].mode;
            shared_temp[GI].partition = shared_temp[GI + 16].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 8)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 8].error )
        {
            shared_temp[GI].error = shared_temp[GI + 8].error;
            shared_temp[GI].mode = shared_temp[GI + 8].mode;
            shared_temp[GI].partition = shared_temp[GI + 8].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 4)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 4].error )
        {
            shared_temp[GI].error = shared_temp[GI + 4].error;
            shared_temp[GI].mode = shared_temp[GI + 4].mode;
            shared_temp[GI].partition = shared_temp[GI + 4].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 2)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 2].error )
        {
            shared_temp[GI].error = shared_temp[GI + 2].error;
            shared_temp[GI].mode = shared_temp[GI + 2].mode;
            shared_temp[GI].partition = shared_temp[GI + 2].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 1)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 1].error )
        {
            shared_temp[GI].error = shared_temp[GI + 1].error;
            shared_temp[GI].mode = shared_temp[GI + 1].mode;
            shared_temp[GI].partition = shared_temp[GI + 1].partition;
        }

        if (g_InBuff[blockID].x > shared_temp[GI].error)
        {
            g_OutBuff[blockID] = uint4(shared_temp[GI].error, shared_temp[GI].mode, shared_temp[GI].partition, 0);
        }
        else
        {
            g_OutBuff[blockID] = g_InBuff[blockID];
        }
    }
}

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void TryMode02CS( uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID )
{
    const uint MAX_USED_THREAD = 64;
    uint BLOCK_IN_GROUP = THREAD_GROUP_SIZE / MAX_USED_THREAD;
    uint blockInGroup = GI / MAX_USED_THREAD;
    uint blockID = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup;
    uint threadBase = blockInGroup * MAX_USED_THREAD;
    uint threadInBlock = GI - threadBase;

    uint block_y = blockID / g_num_block_x;
    uint block_x = blockID - block_y * g_num_block_x;
    uint base_x = block_x * BLOCK_SIZE_X;
    uint base_y = block_y * BLOCK_SIZE_Y;
    
    if (threadInBlock < 16)
    {
        shared_temp[GI].pixel = clamp(uint4(g_Input.Load( uint3( base_x + threadInBlock % 4, base_y + threadInBlock / 4, 0 ) ) * 255), 0, 255);
    }
    GroupMemoryBarrierWithGroupSync();

    shared_temp[GI].error = 0xFFFFFFFF;

    uint num_partitions;
    if (0 == g_mode_id)
    {
        num_partitions = 16;
    }
    else
    {
        num_partitions = 64;
    }

    uint4 pixel_r;
    uint2x4 endPoint[3];
    uint color_index[16];
    if (threadInBlock < num_partitions)
    {
        uint partition = threadInBlock + 64;

        endPoint[0][0] = MAX_UINT;
        endPoint[0][1] = MIN_UINT;
        endPoint[1][0] = MAX_UINT;
        endPoint[1][1] = MIN_UINT;
        endPoint[2][0] = MAX_UINT;
        endPoint[2][1] = MIN_UINT;
        uint bits2 = candidateSectionBit2[partition - 64];
        for ( uint i = 0; i < 16; i ++ )
        {
            uint4 pixel = shared_temp[threadBase + i].pixel;
            if ( (( bits2 >> ( i + 15 ) ) & 0x02) == 2 ) //It gets error when using "candidateSectionCompressed" as "endPoint" index
            {
                endPoint[2][0] = min( endPoint[2][0], pixel );
                endPoint[2][1] = max( endPoint[2][1], pixel );
            }
            else if ( (( bits2 >> i ) & 0x01) == 1 )
            {
                endPoint[1][0] = min( endPoint[1][0], pixel );
                endPoint[1][1] = max( endPoint[1][1], pixel );
            }
            else
            {
                endPoint[0][0] = min( endPoint[0][0], pixel );
                endPoint[0][1] = max( endPoint[0][1], pixel );
            }
        }

        for ( uint i = 0; i < 3; i ++ )
        {
            if (0 == g_mode_id)
            {
                compress_endpoints0( endPoint[i] );
            }
            else
            {
                compress_endpoints2( endPoint[i] );
            }
        }

        uint step_selector = 1 + (2 == g_mode_id);

        int4 span[3];
        span[0] = endPoint[0][1] - endPoint[0][0];
        span[1] = endPoint[1][1] - endPoint[1][0];
        span[2] = endPoint[2][1] - endPoint[2][0];
        span[0].w = span[1].w = span[2].w = 0;
        int span_norm_sqr[3];
        span_norm_sqr[0] = dot( span[0], span[0] );
        span_norm_sqr[1] = dot( span[1], span[1] );
        span_norm_sqr[2] = dot( span[2], span[2] );
        int dotProduct = dot( span[0], shared_temp[threadBase + 0].pixel - endPoint[0][0] );
        if ( span_norm_sqr[0] > 0 && dotProduct > 0 && uint( dotProduct * 63.49999 ) > uint( 32 * span_norm_sqr[0] ) )
        {
            span[0] = -span[0];
            swap(endPoint[0][0], endPoint[0][1]);
        }
        dotProduct = dot( span[1], shared_temp[threadBase + candidateFixUpIndex1D[partition].x].pixel - endPoint[1][0] );
        if ( span_norm_sqr[1] > 0 && dotProduct > 0 && uint( dotProduct * 63.49999 ) > uint( 32 * span_norm_sqr[1] ) )
        {
            span[1] = -span[1];
            swap(endPoint[1][0], endPoint[1][1]);
        }
        dotProduct = dot( span[2], shared_temp[threadBase + candidateFixUpIndex1D[partition].y].pixel - endPoint[2][0] );
        if ( span_norm_sqr[2] > 0 && dotProduct > 0 && uint( dotProduct * 63.49999 ) > uint( 32 * span_norm_sqr[2] ) )
        {
            span[2] = -span[2];
            swap(endPoint[2][0], endPoint[2][1]);
        }

        uint error = 0;
        for ( uint i = 0; i < 16; i ++ )
        {
            if ( (( bits2 >> ( i + 15 ) ) & 0x02) == 2 )
            {
                dotProduct = dot( span[2], shared_temp[threadBase + i].pixel - endPoint[2][0] );
                color_index[i] = ( span_norm_sqr[1] <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr[2] ) ? aStep[step_selector][ uint( dotProduct * 63.49999 / span_norm_sqr[2] ) ] : aStep[step_selector][63] );
            }
            else if ( (( bits2 >> i ) & 0x01) == 1 )
            {
                dotProduct = dot( span[1], shared_temp[threadBase + i].pixel - endPoint[1][0] );
                color_index[i] = ( span_norm_sqr[1] <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr[1] ) ? aStep[step_selector][ uint( dotProduct * 63.49999 / span_norm_sqr[1] ) ] : aStep[step_selector][63] );
            }
            else
            {
                dotProduct = dot( span[0], shared_temp[threadBase + i].pixel - endPoint[0][0] );
                color_index[i] = ( span_norm_sqr[0] <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr[0] ) ? aStep[step_selector][ uint( dotProduct * 63.49999 / span_norm_sqr[0] ) ] : aStep[step_selector][63] );
            }

            uint subset_index = ((bits2 >> (i + 15)) & 0x02) | ((bits2 >> i) & 0x01);
            pixel_r = ( ( 64 - aWeight[step_selector][color_index[i]] ) * endPoint[subset_index][0]
                + aWeight[step_selector][color_index[i]] * endPoint[subset_index][1] + 32 ) >> 6;

            pixel_r -= shared_temp[threadBase + i].pixel;
            pixel_r.w = 0;
            error += dot(pixel_r, pixel_r);
        }

        shared_temp[GI].error = error;
        shared_temp[GI].partition = partition;
    }
    GroupMemoryBarrierWithGroupSync();

    if (threadInBlock < 32)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 32].error )
        {
            shared_temp[GI].error = shared_temp[GI + 32].error;
            shared_temp[GI].partition = shared_temp[GI + 32].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 16)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 16].error )
        {
            shared_temp[GI].error = shared_temp[GI + 16].error;
            shared_temp[GI].partition = shared_temp[GI + 16].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 8)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 8].error )
        {
            shared_temp[GI].error = shared_temp[GI + 8].error;
            shared_temp[GI].partition = shared_temp[GI + 8].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 4)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 4].error )
        {
            shared_temp[GI].error = shared_temp[GI + 4].error;
            shared_temp[GI].partition = shared_temp[GI + 4].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 2)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 2].error )
        {
            shared_temp[GI].error = shared_temp[GI + 2].error;
            shared_temp[GI].partition = shared_temp[GI + 2].partition;
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 1)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 1].error )
        {
            shared_temp[GI].error = shared_temp[GI + 1].error;
            shared_temp[GI].partition = shared_temp[GI + 1].partition;
        }

        if (g_InBuff[blockID].x > shared_temp[GI].error)
        {
            g_OutBuff[blockID] = uint4(shared_temp[GI].error, g_mode_id, shared_temp[GI].partition, 0);
        }
        else
        {
            g_OutBuff[blockID] = g_InBuff[blockID];
        }
    }
}

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void EncodeBlockCS(uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID)
{
    const uint MAX_USED_THREAD = 16;
    uint BLOCK_IN_GROUP = THREAD_GROUP_SIZE / MAX_USED_THREAD;
    uint blockInGroup = GI / MAX_USED_THREAD;
    uint blockID = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup;
    uint threadBase = blockInGroup * MAX_USED_THREAD;
    uint threadInBlock = GI - threadBase;

#ifndef REF_DEVICE
    if (blockID >= g_num_total_blocks)
    {
        return;
    }
#endif

    uint block_y = blockID / g_num_block_x;
    uint block_x = blockID - block_y * g_num_block_x;
    uint base_x = block_x * BLOCK_SIZE_X;
    uint base_y = block_y * BLOCK_SIZE_Y;

    uint mode = g_InBuff[blockID].y & 0x7FFFFFFF;
    uint partition = g_InBuff[blockID].z;
    uint index_selector = (g_InBuff[blockID].y >> 31) & 1;
    uint rotation = g_InBuff[blockID].w;

    if (threadInBlock < 16)
    {
        uint4 pixel = clamp(uint4(g_Input.Load( uint3( base_x + threadInBlock % 4, base_y + threadInBlock / 4, 0 ) ) * 255), 0, 255);

        if (1 == rotation)
        {
            pixel.ra = pixel.ar;
        }
        else if (2 == rotation)
        {
            pixel.ga = pixel.ag;
        }
        else if (3 == rotation)
        {
            pixel.ba = pixel.ab;
        }

        shared_temp[GI].pixel = pixel;
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    uint bits = candidateSectionBit[partition];
    uint bits2 = candidateSectionBit2[partition - 64];

    uint2x4 ep;
    [unroll]
    for (int ii = 2; ii >= 0; -- ii)
    {
        if (threadInBlock < 16)
        {
            uint2x4 ep;
            ep[0] = MAX_UINT;
            ep[1] = MIN_UINT;

            uint4 pixel = shared_temp[GI].pixel;

            if (0 == ii)
            {
                if ((0 == mode) || (2 == mode))
                {
                    if ((((bits2 >> (threadInBlock + 15)) & 0x02) != 2)
                        && (((bits2 >> threadInBlock) & 0x01) != 1))
                    {
                        ep[0] = ep[1] = pixel;
                    }
                }
                else if ((1 == mode) || (3 == mode) || (7 == mode))
                {
                    if ( (( bits >> threadInBlock ) & 0x01) != 1 )
                    {
                        ep[0] = ep[1] = pixel;
                    }
                }
                else if ((4 == mode) || (5 == mode) || (6 == mode))
                {
                    ep[0] = ep[1] = pixel;
                }
            }
            else if (1 == ii)
            {
                if ((0 == mode) || (2 == mode))
                {
                    if ((((bits2 >> (threadInBlock + 15)) & 0x02) != 2)
                        && (((bits2 >> threadInBlock) & 0x01) == 1))
                    {
                        ep[0] = ep[1] = pixel;
                    }
                }
                else if ((1 == mode) || (3 == mode) || (7 == mode))
                {
                    if ( (( bits >> threadInBlock ) & 0x01) == 1 )
                    {
                        ep[0] = ep[1] = pixel;
                    }
                }
            }
            else
            {
                if ((0 == mode) || (2 == mode))
                {
                    if (((bits2 >> (threadInBlock + 15)) & 0x02) == 2)
                    {
                        ep[0] = ep[1] = pixel;
                    }
                }
            }

            shared_temp[GI].endPoint_low = ep[0];
            shared_temp[GI].endPoint_high = ep[1];
        }
#ifdef REF_DEVICE
        GroupMemoryBarrierWithGroupSync();
#endif

        if (threadInBlock < 8)
        {
            shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 8].endPoint_low);
            shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 8].endPoint_high);
        }
#ifdef REF_DEVICE
        GroupMemoryBarrierWithGroupSync();
#endif
        if (threadInBlock < 4)
        {
            shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 4].endPoint_low);
            shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 4].endPoint_high);
        }
#ifdef REF_DEVICE
        GroupMemoryBarrierWithGroupSync();
#endif
        if (threadInBlock < 2)
        {
            shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 2].endPoint_low);
            shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 2].endPoint_high);
        }
#ifdef REF_DEVICE
        GroupMemoryBarrierWithGroupSync();
#endif
        if (threadInBlock < 1)
        {
            shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 1].endPoint_low);
            shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 1].endPoint_high);
        }
#ifdef REF_DEVICE
        GroupMemoryBarrierWithGroupSync();
#endif

        if (ii == threadInBlock)
        {
            ep[0] = shared_temp[threadBase].endPoint_low;
            ep[1] = shared_temp[threadBase].endPoint_high;
        }
    }

    if (threadInBlock < 3)
    {
        if (0 == mode)
        {
            compress_endpoints0( ep );
        }
        else if (1 == mode)
        {
            compress_endpoints1( ep );
        }
        else if (2 == mode)
        {
            compress_endpoints2( ep );
        }
        else if (3 == mode)
        {
            compress_endpoints3( ep );
        }
        else if (4 == mode)
        {
            compress_endpoints4( ep );
        }
        else if (5 == mode)
        {
            compress_endpoints5( ep );
        }
        else if (6 == mode)
        {
            compress_endpoints6( ep );
        }
        else //if (7 == mode)
        {
            compress_endpoints7( ep );
        }

        int4 span = ep[1] - ep[0];
        if (mode < 4)
        {
            span.w = 0;
        }

        if ((4 == mode) || (5 == mode))
        {
            if (0 == threadInBlock)
            {
                int2 span_norm_sqr = uint2( dot( span.rgb, span.rgb ), span.a * span.a );
                int2 dotProduct = int2( dot( span.rgb, shared_temp[threadBase + 0].pixel.rgb - ep[0].rgb ), span.a * ( shared_temp[threadBase + 0].pixel.a - ep[0].a ) );
                if ( span_norm_sqr.x > 0 && dotProduct.x > 0 && uint( dotProduct.x * 63.49999 ) > uint( 32 * span_norm_sqr.x ) )
                {
                    swap(ep[0].rgb, ep[1].rgb);
                }
                if ( span_norm_sqr.y > 0 && dotProduct.y > 0 && uint( dotProduct.y * 63.49999 ) > uint( 32 * span_norm_sqr.y ) )
                {
                    swap(ep[0].a, ep[1].a);
                }
            }
        }
        else //if ((0 == mode) || (2 == mode) || (1 == mode) || (3 == mode) || (7 == mode) || (6 == mode))
        {
            int p;
            if (0 == threadInBlock)
            {
                p = 0;
            }
            else if (1 == threadInBlock)
            {
                p = candidateFixUpIndex1D[partition].x;
            }
            else //if (2 == threadInBlock)
            {
                p = candidateFixUpIndex1D[partition].y;
            }

            int span_norm_sqr = dot( span, span );
            int dotProduct = dot( span, shared_temp[threadBase + p].pixel - ep[0] );
            if ( span_norm_sqr > 0 && dotProduct > 0 && uint( dotProduct * 63.49999 ) > uint( 32 * span_norm_sqr ) )
            {
                swap(ep[0], ep[1]);
            }
        }

        shared_temp[GI].endPoint_low = ep[0];
        shared_temp[GI].endPoint_high = ep[1];
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (threadInBlock < 16)
    {
        uint color_index = 0;
        uint alpha_index = 0;

        uint2x4 ep;

        uint2 indexPrec;
        if ((0 == mode) || (1 == mode))
        {
            indexPrec = 1;
        }
        else if (6 == mode)
        {
            indexPrec = 0;
        }
        else if (4 == mode)
        {
            if (0 == index_selector)
            {
                indexPrec = uint2(2, 1);
            }
            else
            {
                indexPrec = uint2(1, 2);
            }
        }
        else
        {
            indexPrec = 2;
        }

        int subset_index;
        if ((0 == mode) || (2 == mode))
        {
            if ( (( bits2 >> ( threadInBlock + 15 ) ) & 0x02) == 2 )
            {
                subset_index = 2;
            }
            else if ( (( bits2 >> threadInBlock ) & 0x01) == 1 )
            {
                subset_index = 1;
            }
            else
            {
                subset_index = 0;
            }
        }
        else if ((1 == mode) || (3 == mode) || (7 == mode))
        {
            if ( (( bits >> threadInBlock ) & 0x01) == 1 )
            {
                subset_index = 1;
            }
            else
            {
                subset_index = 0;
            }
        }
        else
        {
            subset_index = 0;
        }

        ep[0] = shared_temp[threadBase + subset_index].endPoint_low;
        ep[1] = shared_temp[threadBase + subset_index].endPoint_high;

        int4 span = ep[1] - ep[0];
        if (mode < 4)
        {
            span.w = 0;
        }

        if ((4 == mode) || (5 == mode))
        {
            int2 span_norm_sqr;
            span_norm_sqr.x = dot( span.rgb, span.rgb );
            span_norm_sqr.y = span.a * span.a;
            
            int dotProduct = dot( span.rgb, shared_temp[threadBase + threadInBlock].pixel.rgb - ep[0].rgb );
            color_index = ( span_norm_sqr.x <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr.x ) ? aStep[indexPrec.x][ uint( dotProduct * 63.49999 / span_norm_sqr.x ) ] : aStep[indexPrec.x][63] );
            dotProduct = dot( span.a, shared_temp[threadBase + threadInBlock].pixel.a - ep[0].a );
            alpha_index = ( span_norm_sqr.y <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr.y ) ? aStep[indexPrec.y][ uint( dotProduct * 63.49999 / span_norm_sqr.y ) ] : aStep[indexPrec.y][63] );

            if (index_selector)
            {
                swap(color_index, alpha_index);
            }
        }
        else
        {
            int span_norm_sqr = dot( span, span );

            int dotProduct = dot( span, shared_temp[threadBase + threadInBlock].pixel - ep[0] );
            color_index = ( span_norm_sqr <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr ) ? aStep[indexPrec.x][ uint( dotProduct * 63.49999 / span_norm_sqr ) ] : aStep[indexPrec.x][63] );
        }

        shared_temp[GI].error = color_index;
        shared_temp[GI].mode = alpha_index;
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (0 == threadInBlock)
    {
        uint4 block;
        if (0 == mode)
        {
            block_package0( block, partition, threadBase );
        }
        else if (1 == mode)
        {
            block_package1( block, partition, threadBase );
        }
        else if (2 == mode)
        {
            block_package2( block, partition, threadBase );
        }
        else if (3 == mode)
        {
            block_package3( block, partition, threadBase );
        }
        else if (4 == mode)
        {
            block_package4( block, rotation, index_selector, threadBase );
        }
        else if (5 == mode)
        {
            block_package5( block, rotation, threadBase );
        }
        else if (6 == mode)
        {
            block_package6( block, threadBase );
        }
        else //if (7 == mode)
        {
            block_package7( block, partition, threadBase );
        }

        g_OutBuff[blockID] = block;
    }
}

void compress_endpoints0( inout uint2x4 endPoint )
{
    uint3 tmp;
    for ( uint j = 0; j < 2; j ++ )
    {
        tmp = endPoint[j].rgb & 0x0F;
        tmp.x += tmp.y + tmp.z;
        endPoint[j].rgb = ( endPoint[j].rgb & 0xF0 ) | ( ( tmp.x / 3 ) & 0x08 );
    }
}
void compress_endpoints1( inout uint2x4 endPoint )
{
    uint3 tmp;
    tmp = ( endPoint[0].rgb & 0x03 ) + ( endPoint[1].rgb & 0x03 );
    tmp.x += tmp.y + tmp.z;
    tmp.x = ( tmp.x / 6 ) & 0x02;
    for ( uint j = 0; j < 2; j ++ )
    {
        endPoint[j].rgb = ( endPoint[j].rgb & 0xFC ) | tmp.x;
    }
}
void compress_endpoints2( inout uint2x4 endPoint )
{
    for ( uint j = 0; j < 2; j ++ )
    {
        endPoint[j].rgb = min(255, ( endPoint[j].rgb + 0x04 ) ) & 0xF8;
    }
}
void compress_endpoints3( inout uint2x4 endPoint )
{
    uint3 tmp;
    for ( uint j = 0; j < 2; j ++ )
    {
        tmp = endPoint[j].rgb & 0x01;
        tmp.x += tmp.y + tmp.z;
        endPoint[j].rgb = ( endPoint[j].rgb & 0xFE ) | ( tmp.x / 3 );
    }
}
void compress_endpoints4( inout uint2x4 endPoint )
{
    for ( uint j = 0; j < 2; j ++ )
    {
        endPoint[j] = min(255, ( endPoint[j] + uint4(0x04.xxx, 0x02) ) ) & uint4(0xF8.xxx, 0xFC);
    }
}
void compress_endpoints5( inout uint2x4 endPoint )
{
    for ( uint j = 0; j < 2; j ++ )
    {
        endPoint[j].rgb = min(255, ( endPoint[j].rgb + 0x01 ) ) & 0xFE;
    }
}
void compress_endpoints6( inout uint2x4 endPoint )
{
    uint4 tmp;
    for ( uint j = 0; j < 2; j ++ )
    {
        tmp = endPoint[j] & 0x01;
        tmp.x += tmp.y + tmp.z + tmp.w;
        endPoint[j] = ( endPoint[j] & 0xFE ) | ( ( tmp.x >> 2 ) & 0x01 );
    }
}
void compress_endpoints7( inout uint2x4 endPoint )
{
    uint4 tmp;
    for ( uint j = 0; j < 2; j ++ )
    {
        tmp = endPoint[j] & 0x07;
        tmp.x += tmp.y + tmp.z + tmp.w;
        endPoint[j] = ( endPoint[j] & 0xF8 ) | ( ( tmp.x >> 2 ) & 0x04 );
    }
}

#define get_end_point_l(subset) shared_temp[threadBase + subset].endPoint_low
#define get_end_point_h(subset) shared_temp[threadBase + subset].endPoint_high
#define get_color_index(index) shared_temp[threadBase + index].error
#define get_alpha_index(index) shared_temp[threadBase + index].mode

void block_package0( out uint4 block, uint partition, uint threadBase )
{
    block.x = 0x01 | ( (partition - 64) << 1 ) 
            | ( ( get_end_point_l(0).r & 0xF0 ) <<  1 ) | ( ( get_end_point_h(0).r & 0xF0 ) <<  5 ) 
            | ( ( get_end_point_l(1).r & 0xF0 ) <<  9 ) | ( ( get_end_point_h(1).r & 0xF0 ) << 13 ) 
            | ( ( get_end_point_l(2).r & 0xF0 ) << 17 ) | ( ( get_end_point_h(2).r & 0xF0 ) << 21 ) 
            | ( ( get_end_point_l(0).g & 0xF0 ) << 25 );
    block.y = ( ( get_end_point_l(0).g & 0xF0 ) >>  7 ) | ( ( get_end_point_h(0).g & 0xF0 ) >>  3 ) 
            | ( ( get_end_point_l(1).g & 0xF0 ) <<  1 ) | ( ( get_end_point_h(1).g & 0xF0 ) <<  5 ) 
            | ( ( get_end_point_l(2).g & 0xF0 ) <<  9 ) | ( ( get_end_point_h(2).g & 0xF0 ) << 13 ) 
            | ( ( get_end_point_l(0).b & 0xF0 ) << 17 ) | ( ( get_end_point_h(0).b & 0xF0 ) << 21 )
            | ( ( get_end_point_l(1).b & 0xF0 ) << 25 );
    block.z = ( ( get_end_point_l(1).b & 0xF0 ) >>  7 ) | ( ( get_end_point_h(1).b & 0xF0 ) >>  3 ) 
            | ( ( get_end_point_l(2).b & 0xF0 ) <<  1 ) | ( ( get_end_point_h(2).b & 0xF0 ) <<  5 ) 
            | ( ( get_end_point_l(0).r & 0x08 ) << 10 ) | ( ( get_end_point_h(0).r & 0x08 ) << 11 ) 
            | ( ( get_end_point_l(1).r & 0x08 ) << 12 ) | ( ( get_end_point_h(1).r & 0x08 ) << 13 ) 
            | ( ( get_end_point_l(2).r & 0x08 ) << 14 ) | ( ( get_end_point_h(2).r & 0x08 ) << 15 )
            | ( get_color_index(0) << 19 );
    block.w = 0;
    uint i = 1;
    for ( ; i <= min( candidateFixUpIndex1DOrdered[partition][0], 4 ); i ++ )
    {
        block.z |= get_color_index(i) << ( i * 3 + 18 );
    }
    if ( candidateFixUpIndex1DOrdered[partition][0] < 4 ) //i = 4
    {
        block.z |= get_color_index(4) << 29;
        i += 1;
    }
    else //i = 5
    {
        block.w |= ( get_color_index(4) & 0x04 ) >> 2;
        for ( ; i <= candidateFixUpIndex1DOrdered[partition][0]; i ++ )
            block.w |= get_color_index(i) << ( i * 3 - 14 );
    }
    for ( ; i <= candidateFixUpIndex1DOrdered[partition][1]; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 3 - 15 );
    }
    for ( ; i < 16; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 3 - 16 );
    }
}
void block_package1( out uint4 block, uint partition, uint threadBase )
{
    block.x = 0x02 | ( partition << 2 ) 
            | ( ( get_end_point_l(0).r & 0xFC ) <<  6 ) | ( ( get_end_point_h(0).r & 0xFC ) << 12 ) 
            | ( ( get_end_point_l(1).r & 0xFC ) << 18 ) | ( ( get_end_point_h(1).r & 0xFC ) << 24 );
    block.y = ( ( get_end_point_l(0).g & 0xFC ) >>  2 ) | ( ( get_end_point_h(0).g & 0xFC ) <<  4 ) 
            | ( ( get_end_point_l(1).g & 0xFC ) << 10 ) | ( ( get_end_point_h(1).g & 0xFC ) << 16 )
            | ( ( get_end_point_l(0).b & 0xFC ) << 22 ) | ( ( get_end_point_h(0).b & 0xFC ) << 28 );
    block.z = ( ( get_end_point_h(0).b & 0xFC ) >>  4 ) | ( ( get_end_point_l(1).b & 0xFC ) <<  2 )
            | ( ( get_end_point_h(1).b & 0xFC ) <<  8 ) 
            | ( ( get_end_point_l(0).r & 0x02 ) << 15 ) | ( ( get_end_point_l(1).r & 0x02 ) << 16 )
            | ( get_color_index(0) << 18 );
    if ( candidateFixUpIndex1DOrdered[partition][0] == 15 )
    {
        block.w = (get_color_index(15) << 30) | (get_color_index(14) << 27) | (get_color_index(13) << 24) | (get_color_index(12) << 21) | (get_color_index(11) << 18) | (get_color_index(10) << 15)
            | (get_color_index(9) << 12) | (get_color_index(8) << 9) | (get_color_index(7) << 6) | (get_color_index(6) << 3) | get_color_index(5);
        block.z |= (get_color_index(4) << 29) | (get_color_index(3) << 26) | (get_color_index(2) << 23) | (get_color_index(1) << 20) | (get_color_index(0) << 18);
    }
    else if ( candidateFixUpIndex1DOrdered[partition][0] == 2 )
    {
        block.w = (get_color_index(15) << 29) | (get_color_index(14) << 26) | (get_color_index(13) << 23) | (get_color_index(12) << 20) | (get_color_index(11) << 17) | (get_color_index(10) << 14)
            | (get_color_index(9) << 11) | (get_color_index(8) << 8) | (get_color_index(7) << 5) | (get_color_index(6) << 2) | (get_color_index(5) >> 1);
        block.z |= (get_color_index(5) << 31) | (get_color_index(4) << 28) | (get_color_index(3) << 25) | (get_color_index(2) << 23) | (get_color_index(1) << 20) | (get_color_index(0) << 18);
    }
    else if ( candidateFixUpIndex1DOrdered[partition][0] == 8 )
    {
        block.w = (get_color_index(15) << 29) | (get_color_index(14) << 26) | (get_color_index(13) << 23) | (get_color_index(12) << 20) | (get_color_index(11) << 17) | (get_color_index(10) << 14)
            | (get_color_index(9) << 11) | (get_color_index(8) << 9) | (get_color_index(7) << 6) | (get_color_index(6) << 3) | get_color_index(5);
        block.z |= (get_color_index(4) << 29) | (get_color_index(3) << 26) | (get_color_index(2) << 23) | (get_color_index(1) << 20) | (get_color_index(0) << 18);
    }
    else //candidateFixUpIndex1DOrdered[partition] == 6
    {
        block.w = (get_color_index(15) << 29) | (get_color_index(14) << 26) | (get_color_index(13) << 23) | (get_color_index(12) << 20) | (get_color_index(11) << 17) | (get_color_index(10) << 14)
            | (get_color_index(9) << 11) | (get_color_index(8) << 8) | (get_color_index(7) << 6) | (get_color_index(6) << 4) | get_color_index(5);
        block.z |= (get_color_index(4) << 29) | (get_color_index(3) << 26) | (get_color_index(2) << 23) | (get_color_index(1) << 20) | (get_color_index(0) << 18);
    }
}
void block_package2( out uint4 block, uint partition, uint threadBase )
{
    block.x = 0x04 | ( (partition - 64) << 3 ) 
            | ( ( get_end_point_l(0).r & 0xF8 ) <<  6 ) | ( ( get_end_point_h(0).r & 0xF8 ) << 11 ) 
            | ( ( get_end_point_l(1).r & 0xF8 ) << 16 ) | ( ( get_end_point_h(1).r & 0xF8 ) << 21 ) 
            | ( ( get_end_point_l(2).r & 0xF8 ) << 26 );
    block.y = ( ( get_end_point_l(2).r & 0xF8 ) >>  6 ) | ( ( get_end_point_h(2).r & 0xF8 ) >>  1 )
            | ( ( get_end_point_l(0).g & 0xF8 ) <<  4 ) | ( ( get_end_point_h(0).g & 0xF8 ) <<  9 ) 
            | ( ( get_end_point_l(1).g & 0xF8 ) << 14 ) | ( ( get_end_point_h(1).g & 0xF8 ) << 19 ) 
            | ( ( get_end_point_l(2).g & 0xF8 ) << 24 );
    block.z = ( ( get_end_point_h(2).g & 0xF8 ) >>  3 ) | ( ( get_end_point_l(0).b & 0xF8 ) <<  2 )
            | ( ( get_end_point_h(0).b & 0xF8 ) <<  7 )	| ( ( get_end_point_l(1).b & 0xF8 ) << 12 )
            | ( ( get_end_point_h(1).b & 0xF8 ) << 17 ) | ( ( get_end_point_l(2).b & 0xF8 ) << 22 ) 
            | ( ( get_end_point_h(2).b & 0xF8 ) << 27 );
    block.w = ( ( get_end_point_h(2).b & 0xF8 ) >>  5 ) 
            | ( get_color_index(0) << 3 );
    uint i = 1;
    for ( ; i <= candidateFixUpIndex1DOrdered[partition][0]; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 2 + 2 );
    }
    for ( ; i <= candidateFixUpIndex1DOrdered[partition][1]; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 2 + 1 );
    }
    for ( ; i < 16; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 2 );
    }
}
void block_package3( out uint4 block, uint partition, uint threadBase )
{
    block.x = 0x08 | ( partition << 4 ) 
            | ( ( get_end_point_l(0).r & 0xFE ) <<  9 ) | ( ( get_end_point_h(0).r & 0xFE ) << 16 ) 
            | ( ( get_end_point_l(1).r & 0xFE ) << 23 ) | ( ( get_end_point_h(1).r & 0xFE ) << 30 );
    block.y = ( ( get_end_point_h(1).r & 0xFE ) >>  2 ) | ( ( get_end_point_l(0).g & 0xFE ) <<  5 )
            | ( ( get_end_point_h(0).g & 0xFE ) << 12 ) | ( ( get_end_point_l(1).g & 0xFE ) << 19 )
            | ( ( get_end_point_h(1).g & 0xFE ) << 26 );
    block.z = ( ( get_end_point_h(1).g & 0xFE ) >>  6 ) | ( ( get_end_point_l(0).b & 0xFE ) <<  1 )
            | ( ( get_end_point_h(0).b & 0xFE ) <<  8 ) | ( ( get_end_point_l(1).b & 0xFE ) << 15 )
            | ( ( get_end_point_h(1).b & 0xFE ) << 22 )
            | ( ( get_end_point_l(0).r & 0x01 ) << 30 ) | ( ( get_end_point_h(0).r & 0x01 ) << 31 );
    block.w = ( ( get_end_point_l(1).r & 0x01 ) <<  0 ) | ( ( get_end_point_h(1).r & 0x01 ) <<  1 )
            | ( get_color_index(0) << 2 );
    uint i = 1;
    for ( ; i <= candidateFixUpIndex1DOrdered[partition][0]; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 2 + 1 );
    }
    for ( ; i < 16; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 2 );
    }
}
void block_package4( out uint4 block, uint rotation, uint index_selector, uint threadBase )
{
    block.x = 0x10 | ( rotation << 5 ) | ( index_selector << 7 )
            | ( ( get_end_point_l(0).r & 0xF8 ) <<  5 ) | ( ( get_end_point_h(0).r & 0xF8 ) << 10 )
            | ( ( get_end_point_l(0).g & 0xF8 ) << 15 ) | ( ( get_end_point_h(0).g & 0xF8 ) << 20 )
            | ( ( get_end_point_l(0).b & 0xF8 ) << 25 );
    block.y = ( ( get_end_point_l(0).b & 0xF8 ) >>  7 ) | ( ( get_end_point_h(0).b & 0xF8 ) >>  2 )
            | ( ( get_end_point_l(0).a & 0xFC ) <<  4 ) | ( ( get_end_point_h(0).a & 0xFC ) << 10 )
            | ( get_color_index(0) << 18 ) | ( get_color_index(1) << 19 ) | ( get_color_index(2) << 21 ) | ( get_color_index(3) << 23 ) 
            | ( get_color_index(4) << 25 ) | ( get_color_index(5) << 27 ) | ( get_color_index(6) << 29 ) | ( get_color_index(7) << 31 );
    block.z = ( get_color_index(7) >>  1 ) | ( get_color_index(8) <<  1 ) | ( get_color_index(9) <<  3 ) | ( get_color_index(10)<<  5 )
            | ( get_color_index(11)<<  7 ) | ( get_color_index(12)<<  9 ) | ( get_color_index(13)<< 11 ) | ( get_color_index(14)<< 13 )
            | ( get_color_index(15)<< 15 ) | ( get_alpha_index(0) << 17 ) | ( get_alpha_index(1) << 19 ) | ( get_alpha_index(2) << 22 )
            | ( get_alpha_index(3) << 25 ) | ( get_alpha_index(4) << 28 ) | ( get_alpha_index(5) << 31 );
    block.w = ( get_alpha_index(5) >>  1 ) | ( get_alpha_index(6) <<  2 ) | ( get_alpha_index(7) <<  5 ) | ( get_alpha_index(8) <<  8 ) 
            | ( get_alpha_index(9) << 11 ) | ( get_alpha_index(10)<< 14 ) | ( get_alpha_index(11)<< 17 ) | ( get_alpha_index(12)<< 20 ) 
            | ( get_alpha_index(13)<< 23 ) | ( get_alpha_index(14)<< 26 ) | ( get_alpha_index(15)<< 29 );
}
void block_package5( out uint4 block, uint rotation, uint threadBase )
{
    block.x = 0x20 | ( rotation << 6 )
            | ( ( get_end_point_l(0).r & 0xFE ) <<  7 ) | ( ( get_end_point_h(0).r & 0xFE ) << 14 )
            | ( ( get_end_point_l(0).g & 0xFE ) << 21 ) | ( ( get_end_point_h(0).g & 0xFE ) << 28 );
    block.y = ( ( get_end_point_h(0).g & 0xFE ) >>  4 ) | ( ( get_end_point_l(0).b & 0xFE ) <<  3 )
            | ( ( get_end_point_h(0).b & 0xFE ) << 10 )	| ( get_end_point_l(0).a << 18 ) | ( get_end_point_h(0).a << 26 );
    block.z = ( get_end_point_h(0).a >>  6 )
            | ( get_color_index(0) <<  2 ) | ( get_color_index(1) <<  3 ) | ( get_color_index(2) <<  5 ) | ( get_color_index(3) <<  7 ) 
            | ( get_color_index(4) <<  9 ) | ( get_color_index(5) << 11 ) | ( get_color_index(6) << 13 ) | ( get_color_index(7) << 15 )
            | ( get_color_index(8) << 17 ) | ( get_color_index(9) << 19 ) | ( get_color_index(10)<< 21 ) | ( get_color_index(11)<< 23 ) 
            | ( get_color_index(12)<< 25 ) | ( get_color_index(13)<< 27 ) | ( get_color_index(14)<< 29 ) | ( get_color_index(15)<< 31 );
    block.w =  ( get_color_index(15)>> 1 ) | ( get_alpha_index(0) <<  0 ) | ( get_alpha_index(1) <<  2 ) | ( get_alpha_index(2) <<  4 )
            | ( get_alpha_index(3) <<  6 ) | ( get_alpha_index(4) <<  8 ) | ( get_alpha_index(5) << 10 ) | ( get_alpha_index(6) << 12 )
            | ( get_alpha_index(7) << 14 ) | ( get_alpha_index(8) << 16 ) | ( get_alpha_index(9) << 18 ) | ( get_alpha_index(10)<< 20 ) 
            | ( get_alpha_index(11)<< 22 ) | ( get_alpha_index(12)<< 24 ) | ( get_alpha_index(13)<< 26 ) | ( get_alpha_index(14)<< 28 )
            | ( get_alpha_index(15)<< 30 );
}
void block_package6( out uint4 block, uint threadBase )
{
    block.x = 0x40
            | ( ( get_end_point_l(0).r & 0xFE ) <<  6 ) | ( ( get_end_point_h(0).r & 0xFE ) << 13 )
            | ( ( get_end_point_l(0).g & 0xFE ) << 20 ) | ( ( get_end_point_h(0).g & 0xFE ) << 27 );
    block.y = ( ( get_end_point_h(0).g & 0xFE ) >>  5 ) | ( ( get_end_point_l(0).b & 0xFE ) <<  2 )
            | ( ( get_end_point_h(0).b & 0xFE ) <<  9 )	| ( ( get_end_point_l(0).a & 0xFE ) << 16 )
            | ( ( get_end_point_h(0).a & 0xFE ) << 23 )
            | ( get_end_point_l(0).r & 0x01 ) << 31;
    block.z = ( get_end_point_h(0).r & 0x01 )
            | ( get_color_index(0) <<  1 ) | ( get_color_index(1) <<  4 ) | ( get_color_index(2) <<  8 ) | ( get_color_index(3) << 12 ) 
            | ( get_color_index(4) << 16 ) | ( get_color_index(5) << 20 ) | ( get_color_index(6) << 24 ) | ( get_color_index(7) << 28 );
    block.w = ( get_color_index(8) <<  0 ) | ( get_color_index(9) <<  4 ) | ( get_color_index(10)<<  8 ) | ( get_color_index(11)<< 12 ) 
            | ( get_color_index(12)<< 16 ) | ( get_color_index(13)<< 20 ) | ( get_color_index(14)<< 24 ) | ( get_color_index(15)<< 28 );
}
void block_package7( out uint4 block, uint partition, uint threadBase )
{
    block.x = 0x80 | ( partition << 8 ) 
            | ( ( get_end_point_l(0).r & 0xF8 ) << 11 ) | ( ( get_end_point_h(0).r & 0xF8 ) << 16 ) 
            | ( ( get_end_point_l(1).r & 0xF8 ) << 21 ) | ( ( get_end_point_h(1).r & 0xF8 ) << 26 );
    block.y = ( ( get_end_point_h(1).r & 0xF8 ) >>  6 ) | ( ( get_end_point_l(0).g & 0xF8 ) >>  1 )
            | ( ( get_end_point_h(0).g & 0xF8 ) <<  4 ) | ( ( get_end_point_l(1).g & 0xF8 ) <<  9 ) 
            | ( ( get_end_point_h(1).g & 0xF8 ) << 14 )	| ( ( get_end_point_l(0).b & 0xF8 ) << 19 ) 
            | ( ( get_end_point_h(0).b & 0xF8 ) << 24 );
    block.z = ( ( get_end_point_l(1).b & 0xF8 ) >>  3 )	| ( ( get_end_point_h(1).b & 0xF8 ) <<  2 ) 
            | ( ( get_end_point_l(0).a & 0xF8 ) <<  7 ) | ( ( get_end_point_h(0).a & 0xF8 ) << 12 ) 
            | ( ( get_end_point_l(1).a & 0xF8 ) << 17 ) | ( ( get_end_point_h(1).a & 0xF8 ) << 22 ) 
            | ( ( get_end_point_l(0).r & 0x04 ) << 27 ) | ( ( get_end_point_h(0).r & 0x04 ) << 28 );
    block.w = ( ( get_end_point_l(1).r & 0x04 ) >>  2 ) | ( ( get_end_point_h(1).r & 0x04 ) >>  1 )
            | ( get_color_index(0) <<  2 );
    uint i = 1;
    for ( ; i <= candidateFixUpIndex1DOrdered[partition][0]; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 2 + 1 );
    }
    for ( ; i < 16; i ++ )
    {
        block.w |= get_color_index(i) << ( i * 2 );
    }
}