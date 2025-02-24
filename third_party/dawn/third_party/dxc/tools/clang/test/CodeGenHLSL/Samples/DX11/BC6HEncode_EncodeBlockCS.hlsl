// RUN: %dxc -E main -T cs_6_0 -HV 2018 %s | FileCheck %s

// check input.
// CHECK: flattenedThreadIdInGroup
// CHECK: groupId

// check intrinsic used.
// CHECK: textureLoad
// CHECK: bufferLoad
// CHECK: IMin
// CHECK: IMax
// CHECK: dot3
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: BC6HEncode.hlsl
//
// The Compute Shader for BC6 Decoder
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//#define REF_DEVICE

#define UINTLENGTH			32
#define NCHANNELS			3
#define SIGNED_F16			96
#define	UNSIGNED_F16		95
#define MAX_FLOAT			asfloat(0x7F7FFFFF)
#define MIN_FLOAT			asfloat(0xFF7FFFFF)
#define MAX_INT				asint(0x7FFFFFFF)
#define MIN_INT				asint(0x80000000)

cbuffer cbCS : register( b0 )
{
    uint g_tex_width;
    uint g_num_block_x;
    uint g_format;			//either SIGNED_F16 for DXGI_FORMAT_BC6H_SF16 or UNSIGNED_F16 for DXGI_FORMAT_BC6H_UF16
    uint g_mode_id;
    uint g_start_block_id;
    uint g_num_total_blocks;
};

static const uint candidateModeMemory[14] = { 0x00, 0x01,
    0x02, 0x06, 0x0A, 0x0E, 0x12, 0x16, 0x1A, 0x1E, 0x03, 0x07, 0x0B, 0x0F };
static const uint candidateModeFlag[14] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
static const bool candidateModeTransformed[14] = { true, true, true, true, true, true, true, true, true, false, false, true, true, true };
static const uint4 candidateModePrec[14] = { uint4(10,5,5,5), uint4(7,6,6,6),
    uint4(11,5,4,4), uint4(11,4,5,4), uint4(11,4,4,5), uint4(9,5,5,5),
    uint4(8,6,5,5), uint4(8,5,6,5), uint4(8,5,5,6), uint4(6,6,6,6),
    uint4(10,10,10,10), uint4(11,9,9,9), uint4(12,8,8,8), uint4(16,4,4,4) };

/*static const uint4x4 candidateSection[32] = 
{
    {0,0,1,1, 0,0,1,1, 0,0,1,1, 0,0,1,1}, {0,0,0,1, 0,0,0,1, 0,0,0,1, 0,0,0,1}, {0,1,1,1, 0,1,1,1, 0,1,1,1, 0,1,1,1}, {0,0,0,1, 0,0,1,1, 0,0,1,1, 0,1,1,1},
    {0,0,0,0, 0,0,0,1, 0,0,0,1, 0,0,1,1}, {0,0,1,1, 0,1,1,1, 0,1,1,1, 1,1,1,1}, {0,0,0,1, 0,0,1,1, 0,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,1, 0,0,1,1, 0,1,1,1},
    {0,0,0,0, 0,0,0,0, 0,0,0,1, 0,0,1,1}, {0,0,1,1, 0,1,1,1, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,1, 0,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,0, 0,0,0,1, 0,1,1,1},
    {0,0,0,1, 0,1,1,1, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,0, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 1,1,1,1, 1,1,1,1, 1,1,1,1}, {0,0,0,0, 0,0,0,0, 0,0,0,0, 1,1,1,1},
    {0,0,0,0, 1,0,0,0, 1,1,1,0, 1,1,1,1}, {0,1,1,1, 0,0,0,1, 0,0,0,0, 0,0,0,0}, {0,0,0,0, 0,0,0,0, 1,0,0,0, 1,1,1,0}, {0,1,1,1, 0,0,1,1, 0,0,0,1, 0,0,0,0},
    {0,0,1,1, 0,0,0,1, 0,0,0,0, 0,0,0,0}, {0,0,0,0, 1,0,0,0, 1,1,0,0, 1,1,1,0}, {0,0,0,0, 0,0,0,0, 1,0,0,0, 1,1,0,0}, {0,1,1,1, 0,0,1,1, 0,0,1,1, 0,0,0,1},
    {0,0,1,1, 0,0,0,1, 0,0,0,1, 0,0,0,0}, {0,0,0,0, 1,0,0,0, 1,0,0,0, 1,1,0,0}, {0,1,1,0, 0,1,1,0, 0,1,1,0, 0,1,1,0}, {0,0,1,1, 0,1,1,0, 0,1,1,0, 1,1,0,0},
    {0,0,0,1, 0,1,1,1, 1,1,1,0, 1,0,0,0}, {0,0,0,0, 1,1,1,1, 1,1,1,1, 0,0,0,0}, {0,1,1,1, 0,0,0,1, 1,0,0,0, 1,1,1,0}, {0,0,1,1, 1,0,0,1, 1,0,0,1, 1,1,0,0}
};*/

static const uint candidateSectionBit[32] = 
{
    0xCCCC, 0x8888, 0xEEEE, 0xECC8,
    0xC880, 0xFEEC, 0xFEC8, 0xEC80,
    0xC800, 0xFFEC, 0xFE80, 0xE800,
    0xFFE8, 0xFF00, 0xFFF0, 0xF000,
    0xF710, 0x008E, 0x7100, 0x08CE,
    0x008C, 0x7310, 0x3100, 0x8CCE,
    0x088C, 0x3110, 0x6666, 0x366C,
    0x17E8, 0x0FF0, 0x718E, 0x399C
};

static const uint candidateFixUpIndex1D[32] = 
{
    15,15,15,15,
    15,15,15,15,
    15,15,15,15,
    15,15,15,15,
    15, 2, 8, 2,
     2, 8, 8,15,
     2, 8, 2, 2,
     8, 8, 2, 2
};

//0, 9, 18, 27, 37, 46, 55, 64
static const uint aStep1[64] = {0,0,0,0,0,1,1,1,
                              1,1,1,1,1,1,2,2,
                              2,2,2,2,2,2,2,3,
                              3,3,3,3,3,3,3,3,
                              3,4,4,4,4,4,4,4,
                              4,4,5,5,5,5,5,5,
                              5,5,5,6,6,6,6,6,
                              6,6,6,6,7,7,7,7};
                                  
//0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64
static const uint aStep2[64] = { 0, 0, 0, 1, 1, 1, 1, 2,
                               2, 2, 2, 2, 3, 3, 3, 3,
                               4, 4, 4, 4, 5, 5, 5, 5,
                               6, 6, 6, 6, 6, 7, 7, 7,
                               7, 8, 8, 8, 8, 9, 9, 9,
                               9,10,10,10,10,10,11,11,
                              11,11,12,12,12,12,13,13,
                              13,13,14,14,14,14,15,15};

#define THREAD_GROUP_SIZE	64
#define BLOCK_SIZE_Y		4
#define BLOCK_SIZE_X		4
#define BLOCK_SIZE			(BLOCK_SIZE_Y * BLOCK_SIZE_X)


//Forward declaration
uint3 float2half( float3 pixel_f );
int3 start_quantize( uint3 pixel_h );
void quantize( inout int2x3 endPoint, uint prec );
void finish_quantize_0( inout bool bBadQuantize, inout int2x3 endPoint, uint4 prec, bool transformed );
void finish_quantize_1( inout bool bBadQuantize, inout int2x3 endPoint, uint4 prec, bool transformed );
void finish_quantize( out bool bBadQuantize, inout int2x3 endPoint, uint4 prec, bool transformed );

void start_unquantize( inout int2x3 endPoint[2], uint4 prec, bool transformed );
void start_unquantize( inout int2x3 endPoint, uint4 prec, bool transformed );
void unquantize( inout int2x3 color, uint prec );
uint3 finish_unquantize( int3 color );
void generate_palette_unquantized8( out uint3 palette, int3 low, int3 high, int i );
void generate_palette_unquantized16( out uint3 palette, int3 low, int3 high, int i );
float3 half2float(uint3 color_h );

void block_package( inout uint4 block, int2x3 endPoint[2], uint mode_type, uint partition_index );
void block_package( inout uint4 block, int2x3 endPoint, uint mode_type );

void swap(inout int3 lhs, inout int3 rhs)
{
    int3 tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}

Texture2D<float4> g_Input : register( t0 ); 
StructuredBuffer<uint4> g_InBuff : register( t1 );

RWStructuredBuffer<uint4> g_OutBuff : register( u0 );

struct SharedData
{
    float3 pixel;
    int3 pixel_ph;
    float3 pixel_hr;
    float error;
    uint best_mode;
    uint best_partition;
    int3 endPoint_low;
    int3 endPoint_high;
};

groupshared SharedData shared_temp[THREAD_GROUP_SIZE];

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void TryModeG10CS( uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID )
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
        shared_temp[GI].pixel = g_Input.Load( uint3( base_x + threadInBlock % 4, base_y + threadInBlock / 4, 0 ) ).rgb;
        uint3 pixel_h = float2half( shared_temp[GI].pixel );
        shared_temp[GI].pixel_hr = half2float(pixel_h);
        shared_temp[GI].pixel_ph = start_quantize( pixel_h );
        
        shared_temp[GI].endPoint_low = shared_temp[GI].pixel_ph;
        shared_temp[GI].endPoint_high = shared_temp[GI].pixel_ph;
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

    //ergod mode_type 11:14
    if ( threadInBlock == 0 )
    {
        int2x3 endPoint;
        // find_axis
        endPoint[0] = shared_temp[threadBase + 0].endPoint_low;
        endPoint[1] = shared_temp[threadBase + 0].endPoint_high;
        
        //compute_index
        float3 span = endPoint[1] - endPoint[0];// fixed a bug in v0.2
        float span_norm_sqr = dot( span, span );// fixed a bug in v0.2
        float dotProduct = dot( span, shared_temp[threadBase + 0].pixel_ph - endPoint[0] );// fixed a bug in v0.2
        if ( span_norm_sqr > 0 && dotProduct >= 0 && uint( dotProduct * 63.49999 / span_norm_sqr ) > 32 )
        {
            swap(endPoint[0], endPoint[1]);
            
            shared_temp[GI].endPoint_low = endPoint[0];
            shared_temp[GI].endPoint_high = endPoint[1];
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (threadInBlock < 4)
    {
        int2x3 endPoint;
        endPoint[0] = shared_temp[threadBase + 0].endPoint_low;
        endPoint[1] = shared_temp[threadBase + 0].endPoint_high;
        
        float3 span = endPoint[1] - endPoint[0];
        float span_norm_sqr = dot( span, span );
            
        uint4 prec = candidateModePrec[threadInBlock + 10];
        int2x3 endPoint_q = endPoint;
        quantize( endPoint_q, prec.x );

        bool transformed = candidateModeTransformed[threadInBlock + 10];
        if (transformed)
        {
            endPoint_q[1] -= endPoint_q[0];
        }
        
        bool bBadQuantize;
        finish_quantize( bBadQuantize, endPoint_q, prec, transformed );
        
        start_unquantize( endPoint_q, prec, transformed );
        
        unquantize( endPoint_q, prec.x );
        
        float error = 0;
        [loop]for ( uint j = 0; j < 16; j ++ )
        {
            float dotProduct = dot( span, shared_temp[threadBase + j].pixel_ph - endPoint[0] );// fixed a bug in v0.2
            uint index = ( span_norm_sqr <= 0 || dotProduct <= 0 ) ? 0
                : ( ( dotProduct < span_norm_sqr ) ? aStep2[ uint( dotProduct * 63.49999 / span_norm_sqr ) ] : aStep2[63] );
                
            uint3 pixel_rh;
            generate_palette_unquantized16( pixel_rh, endPoint_q[0], endPoint_q[1], index );
            float3 pixel_r = half2float( pixel_rh );
            pixel_r -= shared_temp[threadBase + j].pixel_hr;
            error += dot(pixel_r, pixel_r);
        }
        if ( bBadQuantize )
            error = 1e20f;

        shared_temp[GI].error = error;
        shared_temp[GI].best_mode = candidateModeFlag[threadInBlock + 10];
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    
    if (threadInBlock < 2)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 2].error )
        {
            shared_temp[GI].error = shared_temp[GI + 2].error;
            shared_temp[GI].best_mode = shared_temp[GI + 2].best_mode;
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
            shared_temp[GI].best_mode = shared_temp[GI + 1].best_mode;
        }
        
        g_OutBuff[blockID] = uint4(asuint(shared_temp[GI].error), shared_temp[GI].best_mode, 0, 0);
    }
}

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void TryModeLE10CS( uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID )
{
    const uint MAX_USED_THREAD = 32;
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

    if (asfloat(g_InBuff[blockID].x) < 1e-6f)
    {
        g_OutBuff[blockID] = g_InBuff[blockID];
        return;
    }
#endif
    
    uint block_y = blockID / g_num_block_x;
    uint block_x = blockID - block_y * g_num_block_x;
    uint base_x = block_x * BLOCK_SIZE_X;
    uint base_y = block_y * BLOCK_SIZE_Y;
    
    if (threadInBlock < 16)
    {
        shared_temp[GI].pixel = g_Input.Load( uint3( base_x + threadInBlock % 4, base_y + threadInBlock / 4, 0 ) ).rgb;
        uint3 pixel_h = float2half( shared_temp[GI].pixel );
        shared_temp[GI].pixel_hr = half2float(pixel_h);
        shared_temp[GI].pixel_ph = start_quantize( pixel_h );
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    
    //ergod mode_type 1:10
    if (threadInBlock < 32)
    {
        int2x3 endPoint[2];
        // find_axis
        endPoint[0][0] = MAX_INT;
        endPoint[0][1] = MIN_INT;
        endPoint[1][0] = MAX_INT;
        endPoint[1][1] = MIN_INT;

        uint bit = candidateSectionBit[threadInBlock];
        for ( uint i = 0; i < 16; i ++ )
        {
            int3 pixel_ph = shared_temp[threadBase + i].pixel_ph;
            if ( (bit >> i) & 1 ) //It gets error when using "candidateSection" as "endPoint_ph" index
            {
                endPoint[1][0] = min( endPoint[1][0], pixel_ph );
                endPoint[1][1] = max( endPoint[1][1], pixel_ph );
            }
            else
            {
                endPoint[0][0] = min( endPoint[0][0], pixel_ph );
                endPoint[0][1] = max( endPoint[0][1], pixel_ph );
            }
        }
        
        //compute_index
        float3 span[2];// fixed a bug in v0.2
        float span_norm_sqr[2];// fixed a bug in v0.2
        [unroll]
        for (uint p = 0; p < 2; ++ p)
        {
            span[p] = endPoint[p][1] - endPoint[p][0];
            span_norm_sqr[p] = dot( span[p], span[p] );

            float dotProduct = dot( span[p], shared_temp[threadBase + (0 == p ? 0 : candidateFixUpIndex1D[threadInBlock])].pixel_ph - endPoint[p][0] );// fixed a bug in v0.2
            if ( span_norm_sqr[p] > 0 && dotProduct >= 0 && uint( dotProduct * 63.49999 / span_norm_sqr[p] ) > 32 )
            {
                span[p] = -span[p];
                swap(endPoint[p][0], endPoint[p][1]);
            }
        }

        uint4 prec = candidateModePrec[g_mode_id];
        int2x3 endPoint_q[2] = endPoint;
        quantize( endPoint_q[0], prec.x );
        quantize( endPoint_q[1], prec.x );

        bool transformed = candidateModeTransformed[g_mode_id];
        if (transformed)
        {
            endPoint_q[0][1] -= endPoint_q[0][0];
            endPoint_q[1][0] -= endPoint_q[0][0];
            endPoint_q[1][1] -= endPoint_q[0][0];
        }

        bool bBadQuantize = false;
        finish_quantize_0( bBadQuantize, endPoint_q[0], prec, transformed );
        finish_quantize_1( bBadQuantize, endPoint_q[1], prec, transformed );
        
        start_unquantize( endPoint_q, prec, transformed );
        
        unquantize( endPoint_q[0], prec.x );
        unquantize( endPoint_q[1], prec.x );
        
        float error = 0;
        for ( uint j = 0; j < 16; j ++ )
        {
            uint3 pixel_rh;
            if ((bit >> j) & 1)
            {
                float dotProduct = dot( span[1], shared_temp[threadBase + j].pixel_ph - endPoint[1][0] );// fixed a bug in v0.2
                uint index = ( span_norm_sqr[1] <= 0 || dotProduct <= 0 ) ? 0
                        : ( ( dotProduct < span_norm_sqr[1] ) ? aStep1[ uint( dotProduct * 63.49999 / span_norm_sqr[1] ) ] : aStep1[63] );
                generate_palette_unquantized8( pixel_rh, endPoint_q[1][0], endPoint_q[1][1], index );
            }
            else
            {
                float dotProduct = dot( span[0], shared_temp[threadBase + j].pixel_ph - endPoint[0][0] );// fixed a bug in v0.2
                uint index = ( span_norm_sqr[0] <= 0 || dotProduct <= 0 ) ? 0
                        : ( ( dotProduct < span_norm_sqr[0] ) ? aStep1[ uint( dotProduct * 63.49999 / span_norm_sqr[0] ) ] : aStep1[63] );
                generate_palette_unquantized8( pixel_rh, endPoint_q[0][0], endPoint_q[0][1], index );
            }

            float3 pixel_r = half2float( pixel_rh );
            pixel_r -= shared_temp[threadBase + j].pixel_hr;
            error += dot(pixel_r, pixel_r);
        }
        if ( bBadQuantize )
            error = 1e20f;

        shared_temp[GI].error = error;
        shared_temp[GI].best_mode = candidateModeFlag[g_mode_id];
        shared_temp[GI].best_partition = threadInBlock;
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    
    if (threadInBlock < 16)
    {
        if ( shared_temp[GI].error > shared_temp[GI + 16].error )
        {
            shared_temp[GI].error = shared_temp[GI + 16].error;
            shared_temp[GI].best_mode = shared_temp[GI + 16].best_mode;
            shared_temp[GI].best_partition = shared_temp[GI + 16].best_partition;
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
            shared_temp[GI].best_mode = shared_temp[GI + 8].best_mode;
            shared_temp[GI].best_partition = shared_temp[GI + 8].best_partition;
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
            shared_temp[GI].best_mode = shared_temp[GI + 4].best_mode;
            shared_temp[GI].best_partition = shared_temp[GI + 4].best_partition;
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
            shared_temp[GI].best_mode = shared_temp[GI + 2].best_mode;
            shared_temp[GI].best_partition = shared_temp[GI + 2].best_partition;
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
            shared_temp[GI].best_mode = shared_temp[GI + 1].best_mode;
            shared_temp[GI].best_partition = shared_temp[GI + 1].best_partition;
        }
        
        if (asfloat(g_InBuff[blockID].x) > shared_temp[GI].error)
        {
            g_OutBuff[blockID] = uint4(asuint(shared_temp[GI].error), shared_temp[GI].best_mode, shared_temp[GI].best_partition, 0);
        }
        else
        {
            g_OutBuff[blockID] = g_InBuff[blockID];
        }
    }
}

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void main(uint GI : SV_GroupIndex, uint3 groupID : SV_GroupID)
{
    const uint MAX_USED_THREAD = 32;
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
        shared_temp[GI].pixel = g_Input.Load( uint3( base_x + threadInBlock % 4, base_y + threadInBlock / 4, 0 ) ).rgb;
        uint3 pixel_h = float2half( shared_temp[GI].pixel );
        shared_temp[GI].pixel_ph = start_quantize( pixel_h );
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    
    uint best_mode = g_InBuff[blockID].y;
    uint best_partition = g_InBuff[blockID].z;
    
    uint4 block = 0;

    if (threadInBlock < 32)
    {
        int2x3 endPoint;
        endPoint[0] = MAX_INT;
        endPoint[1] = MIN_INT;

        int3 pixel_ph = shared_temp[threadBase + (threadInBlock & 0xF)].pixel_ph;
        if (threadInBlock < 16)
        {
            if (best_mode > 10)
            {
                endPoint[0] = endPoint[1] = pixel_ph;
            }
            else
            {
                uint bits = candidateSectionBit[best_partition];
                if (0 == ((bits >> threadInBlock) & 1))
                {
                    endPoint[0] = endPoint[1] = pixel_ph;
                }
            }
        }
        else
        {
            if (best_mode <= 10)
            {
                uint bits = candidateSectionBit[best_partition];
                if (1 == ((bits >> (threadInBlock & 0xF)) & 1))
                {
                    endPoint[0] = endPoint[1] = pixel_ph;
                }
            }
        }
        
        shared_temp[GI].endPoint_low = endPoint[0];
        shared_temp[GI].endPoint_high = endPoint[1];
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if ((threadInBlock & 0xF) < 8)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 8].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 8].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if ((threadInBlock & 0xF) < 4)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 4].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 4].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if ((threadInBlock & 0xF) < 2)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 2].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 2].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if ((threadInBlock & 0xF) < 1)
    {
        shared_temp[GI].endPoint_low = min(shared_temp[GI].endPoint_low, shared_temp[GI + 1].endPoint_low);
        shared_temp[GI].endPoint_high = max(shared_temp[GI].endPoint_high, shared_temp[GI + 1].endPoint_high);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (threadInBlock < 2)
    {
        // find_axis
        int2x3 endPoint;
        endPoint[0] = shared_temp[threadBase + threadInBlock * 16].endPoint_low;
        endPoint[1] = shared_temp[threadBase + threadInBlock * 16].endPoint_high;

        uint fixup = 0;
        if ((1 == threadInBlock) && (best_mode <= 10))
        {
            fixup = candidateFixUpIndex1D[best_partition];
        }
        
        float3 span = endPoint[1] - endPoint[0];
        float span_norm_sqr = dot( span, span );
        float dotProduct = dot( span, shared_temp[threadBase + fixup].pixel_ph - endPoint[0] );
        if ( span_norm_sqr > 0 && dotProduct >= 0 && uint( dotProduct * 63.49999 / span_norm_sqr ) > 32 )
        {
            swap(endPoint[0], endPoint[1]);
        }

        shared_temp[GI].endPoint_low = endPoint[0];
        shared_temp[GI].endPoint_high = endPoint[1];
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    
    if (threadInBlock < 16)
    {
        uint bits;
        if (best_mode > 10)
        {
            bits = 0;
        }
        else
        {
            bits = candidateSectionBit[best_partition];
        }

        float3 span;
        float dotProduct;
        if ((bits >> threadInBlock) & 1)
        {
            span = shared_temp[threadBase + 1].endPoint_high - shared_temp[threadBase + 1].endPoint_low;
            dotProduct = dot( span, shared_temp[threadBase + threadInBlock].pixel_ph - shared_temp[threadBase + 1].endPoint_low );
        }
        else
        {
            span = shared_temp[threadBase + 0].endPoint_high - shared_temp[threadBase + 0].endPoint_low;
            dotProduct = dot( span, shared_temp[threadBase + threadInBlock].pixel_ph - shared_temp[threadBase + 0].endPoint_low );
        }
        float span_norm_sqr = dot( span, span );

        if (best_mode > 10)
        {
            uint index = ( span_norm_sqr <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr ) ? aStep2[ uint( dotProduct * 63.49999 / span_norm_sqr ) ] : aStep2[63] );
            if (threadInBlock == 0)
            {
                block.z |= index << 1;
            }
            else if (threadInBlock < 8)
            {
                block.z |= index << (threadInBlock * 4);
            }
            else
            {
                block.w |= index << ((threadInBlock - 8) * 4);
            }
        }
        else
        {
            uint index = ( span_norm_sqr <= 0 || dotProduct <= 0 ) ? 0
                    : ( ( dotProduct < span_norm_sqr ) ? aStep1[ uint( dotProduct * 63.49999 / span_norm_sqr ) ] : aStep1[63] );

            uint fixup = candidateFixUpIndex1D[best_partition];
            int2 offset = int2((fixup != 2), (fixup == 15));

            if (threadInBlock == 0)
            {
                block.z |= index << 18;
            }
            else if (threadInBlock < 3)
            {
                block.z |= index << (20 + (threadInBlock - 1) * 3);
            }
            else if (threadInBlock < 5)
            {
                block.z |= index << (25 + (threadInBlock - 3) * 3 + offset.x);
            }
            else if (threadInBlock == 5)
            {
                block.w |= index >> !offset.x;
                if (!offset.x)
                {
                    block.z |= index << 31;
                }
            }
            else if (threadInBlock < 9)
            {
                block.w |= index << (2 + (threadInBlock - 6) * 3 + offset.x);
            }
            else
            {
                block.w |= index << (11 + (threadInBlock - 9) * 3 + offset.y);
            }
        }
        
        shared_temp[GI].pixel_hr.xy = asfloat(block.zw);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 8)
    {
        shared_temp[GI].pixel_hr.xy = asfloat(asuint(shared_temp[GI].pixel_hr.xy) | asuint(shared_temp[GI + 8].pixel_hr.xy));
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 4)
    {
        shared_temp[GI].pixel_hr.xy = asfloat(asuint(shared_temp[GI].pixel_hr.xy) | asuint(shared_temp[GI + 4].pixel_hr.xy));
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 2)
    {
        shared_temp[GI].pixel_hr.xy = asfloat(asuint(shared_temp[GI].pixel_hr.xy) | asuint(shared_temp[GI + 2].pixel_hr.xy));
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock < 1)
    {
        shared_temp[GI].pixel_hr.xy = asfloat(asuint(shared_temp[GI].pixel_hr.xy) | asuint(shared_temp[GI + 1].pixel_hr.xy));
        
        block.zw = asuint(shared_temp[GI].pixel_hr.xy);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    bool transformed = candidateModeTransformed[best_mode - 1];
    uint4 prec = candidateModePrec[best_mode - 1];
    if (threadInBlock == 2)
    {
        int2x3 endPoint_q;
        endPoint_q[0] = shared_temp[threadBase + 0].endPoint_low;
        endPoint_q[1] = shared_temp[threadBase + 0].endPoint_high;

        quantize( endPoint_q, prec.x );
        if (transformed)
        {
            endPoint_q[1] -= endPoint_q[0];
        }

        shared_temp[GI].endPoint_low = endPoint_q[0];
        shared_temp[GI].endPoint_high = endPoint_q[1];
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    if (threadInBlock == 3)
    {
        int3 ep0 = shared_temp[threadBase + 2].endPoint_low;
        int2x3 endPoint_q;
        endPoint_q[0] = shared_temp[threadBase + 1].endPoint_low;
        endPoint_q[1] = shared_temp[threadBase + 1].endPoint_high;

        if (best_mode <= 10)
        {
            quantize( endPoint_q, prec.x );
            if (transformed)
            {
                endPoint_q[0] -= ep0;
                endPoint_q[1] -= ep0;
            }

            shared_temp[GI].endPoint_low = endPoint_q[0];
            shared_temp[GI].endPoint_high = endPoint_q[1];
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (threadInBlock < 2)
    {
        int2x3 endPoint_q;
        endPoint_q[0] = shared_temp[threadBase + threadInBlock + 2].endPoint_low;
        endPoint_q[1] = shared_temp[threadBase + threadInBlock + 2].endPoint_high;

        bool bBadQuantize = false;
        if (threadInBlock == 0)
        {
            if (best_mode > 10)
            {
                finish_quantize( bBadQuantize, endPoint_q, prec, transformed );
            }
            else
            {
                finish_quantize_0( bBadQuantize, endPoint_q, prec, transformed );
            }
        }
        else // if (threadInBlock == 1)
        {
            if (best_mode <= 10)
            {
                finish_quantize_1( bBadQuantize, endPoint_q, prec, transformed );
            }
        }

        shared_temp[GI].endPoint_low = endPoint_q[0];
        shared_temp[GI].endPoint_high = endPoint_q[1];
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif
    
    if ( threadInBlock == 0 )
    {
        int2x3 endPoint_q[2];
        endPoint_q[0][0] = shared_temp[threadBase + 0].endPoint_low;
        endPoint_q[0][1] = shared_temp[threadBase + 0].endPoint_high;
        endPoint_q[1][0] = shared_temp[threadBase + 1].endPoint_low;
        endPoint_q[1][1] = shared_temp[threadBase + 1].endPoint_high;

        if ( best_mode > 10 )
        {
            block_package( block, endPoint_q[0], best_mode );
        }
        else
        {
            block_package( block, endPoint_q, best_mode, best_partition );
        }
        
        g_OutBuff[blockID] = block;
    }
}

uint3 float2half( float3 endPoint_f )
{
    uint3 sign = asuint(endPoint_f) & 0x80000000;
    uint3 expo = asuint(endPoint_f) & 0x7F800000;
    uint3 base = asuint(endPoint_f) & 0x007FFFFF;
    return ( expo < 0x33800000 ) ? 0 
                        //0x33800000 indicating 2^-24, which is minimal denormalized number that half can present 
        : ( ( expo < 0x38800000 ) ? ( sign >> 16 ) | ( ( base + 0x00800000 ) >> ( 23 - ( ( expo - 0x33800000 ) >> 23 ) ) )//fixed a bug in v0.2
                        //0x38800000 indicating 2^-14, which is minimal normalized number that half can present, so need to use denormalized half presentation
        : ( ( expo == 0x7F800000 || expo > 0x47000000 ) ? ( ( sign >> 16 ) | 0x7bff )
                        // treat NaN as INF, treat INF (including NaN) as the maximum/minimum number that half can present
                        // 0x47000000 indicating 2^15, which is maximum exponent that half can present, so cut to 0x7bff which is the maximum half number
        : ( ( sign >> 16 ) | ( ( ( expo - 0x38000000 ) | base ) >> 13 ) ) ) );
}
int3 start_quantize( uint3 pixel_h )
{
    if ( g_format == UNSIGNED_F16 )
    {
        return asint( ( pixel_h << 6 ) / 31 );
    }
    else
    {
        return ( pixel_h < 0x8000 ) ? ( ( pixel_h == 0x7bff ) ? 0x7fff : asint( ( pixel_h << 5 ) / 31 ) )// fixed a bug in v0.2
            : ( ( pixel_h == 0x7bff ) ? 0xffff8001 : -asint( ( ( 0x00007fff & pixel_h ) << 5 ) / 31 ) );// fixed a bug in v0.2
    }
}
void quantize( inout int2x3 endPoint, uint prec )
{
    int iprec = asint( prec );
    if ( g_format == UNSIGNED_F16 )
    {
        endPoint = ( ( iprec >= 15 ) | ( endPoint == 0 ) ) ? endPoint
            : ( ( endPoint == asint(0xFFFF) ) ? ( ( 1 << iprec ) - 1 )
            : ( ( ( endPoint << iprec ) + asint(0x0000) ) >> 16 ) );
    }
    else
    {
        endPoint = ( ( iprec >= 16 ) | ( endPoint == 0 ) ) ? endPoint
            : ( ( endPoint >= 0 ) ? ( ( endPoint == asint(0x7FFF) ) ? ( ( 1 << ( iprec - 1 ) ) - 1 ) : ( ( ( endPoint << ( iprec - 1 ) ) + asint(0x0000) ) >> 15 ) ) 
            : ( ( -endPoint == asint(0x7FFF) ) ? -( ( 1 << ( iprec - 1 ) ) - 1 ) : -( ( ( -endPoint << ( iprec - 1 ) ) + asint(0x0000) ) >> 15 ) ) );
    }
}
void finish_quantize_0( inout bool bBadQuantize, inout int2x3 endPoint, uint4 prec, bool transformed )
{
    if ( transformed )
    {
        bool3 bBadComponent = ( endPoint[1] >= 0 ) ? ( ( endPoint[1] >= ( 1 << ( prec.yzw - 1 ) ) ) ? true : false )
            : ( ( -endPoint[1] > ( 1 << ( prec.yzw - 1 ) ) ) ? true : false );
        bBadQuantize = bBadQuantize || any(bBadComponent);

        endPoint[0] = endPoint[0] & ( ( 1 << prec.x ) - 1 );
        endPoint[1] = ( endPoint[1] >= 0 ) ? ( ( endPoint[1] >= ( 1 << ( prec.yzw - 1 ) ) ) ? ( ( 1 << ( prec.yzw - 1 ) ) - 1 ) : endPoint[1] )
            : ( ( -endPoint[1] > ( 1 << ( prec.yzw - 1 ) ) ) ? ( 1 << ( prec.yzw - 1 ) ) : ( endPoint[1] & ( ( 1 << prec.yzw ) - 1 ) ) );
    }
    else
    {
        endPoint &= ( ( 1 << prec.x ) - 1 );
    }
}
void finish_quantize_1( inout bool bBadQuantize, inout int2x3 endPoint, uint4 prec, bool transformed )
{
    if ( transformed )
    {
        bool2x3 bBadComponent;
        bBadComponent[0] = ( endPoint[0] >= 0 ) ? ( ( endPoint[0] >= ( 1 << ( prec.yzw - 1 ) ) ) ? true : false )
            : ( ( -endPoint[0] > ( 1 << ( prec.yzw - 1 ) ) ) ? true : false );
        bBadComponent[1] = ( endPoint[1] >= 0 ) ? ( ( endPoint[1] >= ( 1 << ( prec.yzw - 1 ) ) ) ? true : false )
            : ( ( -endPoint[1] > ( 1 << ( prec.yzw - 1 ) ) ) ? true : false );
        bBadQuantize = bBadQuantize || any(bBadComponent);

        endPoint[0] = ( endPoint[0] >= 0 ) ? ( ( endPoint[0] >= ( 1 << ( prec.yzw - 1 ) ) ) ? ( ( 1 << ( prec.yzw - 1 ) ) - 1 ) : endPoint[0] )
            : ( ( -endPoint[0] > ( 1 << ( prec.yzw - 1 ) ) ) ? ( 1 << ( prec.yzw - 1 ) ) : ( endPoint[0] & ( ( 1 << prec.yzw ) - 1 ) ) );
        endPoint[1] = ( endPoint[1] >= 0 ) ? ( ( endPoint[1] >= ( 1 << ( prec.yzw - 1 ) ) ) ? ( ( 1 << ( prec.yzw - 1 ) ) - 1 ) : endPoint[1] )
            : ( ( -endPoint[1] > ( 1 << ( prec.yzw - 1 ) ) ) ? ( 1 << ( prec.yzw - 1 ) ) : ( endPoint[1] & ( ( 1 << prec.yzw ) - 1 ) ) );
    }
    else
    {
        endPoint &= ( ( 1 << prec.x ) - 1 );
    }
}
void finish_quantize( out bool bBadQuantize, inout int2x3 endPoint, uint4 prec, bool transformed )
{
    if ( transformed )
    {
        bool3 bBadComponent;
        bBadComponent = ( endPoint[1] >= 0 ) ? ( ( endPoint[1] >= ( 1 << ( prec.yzw - 1 ) ) ) ? true : false )
            : ( ( -endPoint[1] > ( 1 << ( prec.yzw - 1 ) ) ) ? true : false );
        bBadQuantize = any( bBadComponent );

        endPoint[0] = endPoint[0] & ( ( 1 << prec.x ) - 1 );
        endPoint[1] = ( endPoint[1] >= 0 ) ? ( ( endPoint[1] >= ( 1 << ( prec.yzw - 1 ) ) ) ? ( ( 1 << ( prec.yzw - 1 ) ) - 1 ) : endPoint[1] )
            : ( ( -endPoint[1] > ( 1 << ( prec.yzw - 1 ) ) ) ? ( 1 << ( prec.yzw - 1 ) ) : ( endPoint[1] & ( ( 1 << prec.yzw ) - 1 ) ) );            
    }
    else
    {
        endPoint &= ( ( 1 << prec.x ) - 1 );
        
        bBadQuantize = false;
    }
}

void SIGN_EXTEND( uint3 prec, inout int3 color )
{
    uint3 p = 1 << (prec - 1);
    color = (color & p) ? (color & (p - 1)) - p : color;
}

void sign_extend( bool transformed, uint4 prec, inout int2x3 endPoint )
{
    if ( g_format == SIGNED_F16 )
        SIGN_EXTEND( prec.x, endPoint[0] );
    if ( g_format == SIGNED_F16 || transformed )
        SIGN_EXTEND( prec.yzw, endPoint[1] );
}

void sign_extend( bool transformed, uint4 prec, inout int2x3 endPoint[2] )
{
    if ( g_format == SIGNED_F16 )
        SIGN_EXTEND( prec.x, endPoint[0][0] );
    if ( g_format == SIGNED_F16 || transformed )
    {
        SIGN_EXTEND( prec.yzw, endPoint[0][1] );
        SIGN_EXTEND( prec.yzw, endPoint[1][0] );
        SIGN_EXTEND( prec.yzw, endPoint[1][1] );
    }
}
void start_unquantize( inout int2x3 endPoint[2], uint4 prec, bool transformed )
{
    sign_extend( transformed, prec, endPoint );
    if ( transformed )
    {
        endPoint[0][1] += endPoint[0][0];
        endPoint[1][0] += endPoint[0][0];
        endPoint[1][1] += endPoint[0][0];
    }
}
void start_unquantize( inout int2x3 endPoint, uint4 prec, bool transformed )
{
    sign_extend( transformed, prec, endPoint );
    if ( transformed )
        endPoint[1] += endPoint[0];
}
void unquantize( inout int2x3 color, uint prec )
{
    int iprec = asint( prec );
    if (g_format == UNSIGNED_F16 )
    {
        if (prec < 15)
        {
            color = (color != 0) ? (color == ((1 << iprec) - 1) ? 0xFFFF : (((color << 16) + 0x8000) >> iprec)) : color;
        }
    }
    else
    {
        if (prec < 16)
        {
            uint2x3 s = color >= 0 ? 0 : 1;
            color = abs(color);
            color = (color != 0) ? (color >= ((1 << (iprec - 1)) - 1) ? 0x7FFF : (((color << 15) + 0x4000) >> (iprec - 1))) : color;
            color = s > 0 ? -color : color;
        }
    }
}
uint3 finish_unquantize( int3 color )
{
    if ( g_format == UNSIGNED_F16 )
        color = ( color * 31 ) >> 6;
    else
    {
        color = ( color < 0 ) ? -( ( -color * 31 ) >> 5 ) : ( color * 31 ) >> 5;
        color = ( color < 0 ) ? ( ( -color ) | 0x8000 ) : color;
    }
    return asuint(color);
}
void generate_palette_unquantized8( out uint3 palette, int3 low, int3 high, int i )
{
    static const int aWeight3[] = {0, 9, 18, 27, 37, 46, 55, 64};
    
    int3 tmp = ( low * ( 64 - aWeight3[i] ) + high * aWeight3[i] + 32 ) >> 6;
    palette = finish_unquantize( tmp );
}
void generate_palette_unquantized16( out uint3 palette, int3 low, int3 high, int i )
{
    static const int aWeight4[] = {0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};
    
    int3 tmp = ( low * ( 64 - aWeight4[i] ) + high * aWeight4[i] + 32 ) >> 6;
    palette = finish_unquantize( tmp );
}
float3 half2float(uint3 color_h )
{
    uint3 sign = color_h & 0x8000;
    uint3 expo = color_h & 0x7C00;
    uint3 base = color_h & 0x03FF;
    return ( expo == 0 ) ? asfloat( ( sign << 16 ) | asuint( float3(base) / 16777216 ) ) //16777216 = 2^24
        : asfloat( ( sign << 16 ) | ( ( ( expo + 0x1C000 ) | base ) << 13 ) ); //0x1C000 = 0x1FC00 - 0x3C00
}

void block_package( inout uint4 block, int2x3 endPoint[2], uint mode_type, uint partition_index )
{
    block.z |= partition_index << 13;
    
    if ( mode_type == candidateModeFlag[0])
    {
        block.x = candidateModeMemory[0];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00007FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x01FF8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.x |= ( endPoint[1][0].g >> 2 ) & 0x00000004;
        block.x |= ( endPoint[1][0].b >> 1 ) & 0x00000008;
        block.x |= endPoint[1][1].b & 0x00000010;
        block.y |= ( ( endPoint[0][0].b >> 7 ) & 0x00000007 );
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000000F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0003E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x0F800000 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000003E);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.y |= ( ( endPoint[1][1].g << 4 ) & 0x00000100 );
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00000F80);
        block.yz |= ( ( endPoint[1][1].b << uint2(27, 9) ) & uint2(0x10000000, 0x00001000) ) | ( ( endPoint[1][1].b << uint2(18, 4) ) & uint2(0x00040000, 0x00000040) );
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
    }
    else if ( mode_type == candidateModeFlag[1])
    {
        block.x = candidateModeMemory[1];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00000FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x003F8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000001F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0007E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x1F800000 );
        block.x |= ( ( endPoint[1][0].g >> 3 ) & 0x00000004 ) | ( ( endPoint[1][0].g << 20 ) & 0x01000000 );
        block.x |= ( endPoint[1][1].g >> 1 ) & 0x00000018;
        block.x |= ( ( endPoint[1][1].b << 21 ) & 0x00800000 ) | ( ( endPoint[1][1].b << 12 ) & 0x00003000 );
        block.x |= ( ( endPoint[1][0].b << 17 ) & 0x00400000 ) | ( ( endPoint[1][0].b << 10 ) & 0x00004000 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000007E);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00001F80);
        block.y |= ( ( endPoint[1][1].b >> 4 ) & 0x00000002 ) | ( ( endPoint[1][1].b >> 2 ) & 0x00000004 ) | ( ( endPoint[1][1].b >> 3 ) & 0x00000001 );
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
    }
    else if ( mode_type == candidateModeFlag[2])
    {
        block.x = candidateModeMemory[2];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00007FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x01FF8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.y |= ( endPoint[0][0].r >> 2 ) & 0x00000100;
        block.y |= ( endPoint[0][0].g << 7 ) & 0x00020000;
        block.y |= ( ( endPoint[0][0].b << 17 ) & 0x08000000 ) | ( ( endPoint[0][0].b >> 7 ) & 0x00000007 );
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000000F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0001E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x07800000 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000003E);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00000F80);
        block.yz |= ( ( endPoint[1][1].b << uint2(27, 9) ) & uint2(0x10000000, 0x00001000) ) | ( ( endPoint[1][1].b << uint2(18, 4) ) & uint2(0x00040000, 0x00000040) );
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
    }
    else if ( mode_type == candidateModeFlag[3])
    {
        block.x = candidateModeMemory[3];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00007FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x01FF8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.y |= ( endPoint[0][0].r >> 3 ) & 0x00000080;
        block.y |= ( endPoint[0][0].g << 8 ) & 0x00040000;
        block.y |= ( ( endPoint[0][0].b << 17 ) & 0x08000000 ) | ( ( endPoint[0][0].b >> 7 ) & 0x00000007 );
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x00000078 ) | ( ( endPoint[0][1].g << 13 ) & 0x0003E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x07800000 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000001E);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.y |= ( ( endPoint[1][1].g << 4 ) & 0x00000100 );
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00000780);
        block.yz |= ( endPoint[1][1].b << uint2(27, 9) ) & uint2(0x10000000, 0x00001000);
        block.z |= ( ( endPoint[1][0].g << 7 ) & 0x00000800 );
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
        block.z |= ( endPoint[1][1].b << 4 ) & 0x00000040;
        block.z |= ( endPoint[1][1].b << 5 ) & 0x00000020;
    }
    else if ( mode_type == candidateModeFlag[4])
    {
        block.x = candidateModeMemory[4];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00007FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x01FF8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.y |= ( endPoint[0][0].r >> 3 ) & 0x00000080;
        block.y |= ( endPoint[0][0].g << 7 ) & 0x00020000;
        block.y |= ( ( endPoint[0][0].b << 18 ) & 0x10000000 ) | ( ( endPoint[0][0].b >> 7 ) & 0x00000007 );
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x00000078 ) | ( ( endPoint[0][1].g << 13 ) & 0x0001E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x0F800000 );
        block.y |= ( ( endPoint[1][0].g << 9 ) & 0x00001E00 ) | ( ( endPoint[1][0].b << 4 ) & 0x00000100 );
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00000780);
        block.yz |= ( endPoint[1][1].b << uint2(18, 4) ) & uint2(0x00040000, 0x00000060);
        block.z |= ( endPoint[1][0].r << 1 ) & 0x0000001E;
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
        block.z |= ( ( endPoint[1][1].b << 7 ) & 0x00000800 ) | ( ( endPoint[1][1].b << 9 ) & 0x00001000 );
    }
    else if ( mode_type == candidateModeFlag[5])
    {
        block.x = candidateModeMemory[5];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00003FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x00FF8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000);
        block.y |= ( endPoint[0][0].b >> 7 ) & 0x00000003;
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000000F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0003E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x0F800000 );
        block.x |= ( ( endPoint[1][0].g << 20 ) & 0x01000000 ) | ( ( endPoint[1][0].b << 10 ) & 0x00004000 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000003E);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.y |= ( ( endPoint[1][1].g << 4 ) & 0x00000100 ) | ( ( endPoint[1][1].b >> 2 ) & 0x00000004 );
        block.y |= ( ( endPoint[1][1].b << 27 ) & 0x10000000 );
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00000F80);
        block.yz |= ( endPoint[1][1].b << uint2(18, 4) ) & uint2(0x00040000, 0x00000040);
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
        block.z |= ( ( endPoint[1][1].b << 9 ) & 0x00001000 );
    }
    else if ( mode_type == candidateModeFlag[6])
    {
        block.x = candidateModeMemory[6];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00001FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x007F8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.y |= ( endPoint[0][0].b >> 7 ) & 0x00000001;
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000001F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0003E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x0F800000 );
        block.x |= ( ( endPoint[1][0].g << 20 ) & 0x01000000 ) | ( ( endPoint[1][0].b << 10 ) & 0x00004000);
        block.x |= ( ( endPoint[1][1].g << 9 ) & 0x00002000 ) | ( ( endPoint[1][1].b << 21 ) & 0x00800000);
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000007E);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00001F80);
        block.y |= ( ( endPoint[1][1].b >> 2 ) & 0x00000006 );
        block.y |= ( ( endPoint[1][1].b << 27 ) & 0x10000000 ) | ( ( endPoint[1][1].b << 18 ) & 0x00040000 );
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
    }
    else if ( mode_type == candidateModeFlag[7])
    {
        block.x = candidateModeMemory[7];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00001FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x007F8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.y |= ( endPoint[0][0].b >> 7 ) & 0x00000001;
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000000F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0007E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x0F800000 );
        block.x |= ( ( endPoint[1][0].g << 20 ) & 0x01000000 ) | ( ( endPoint[1][0].b << 10 ) & 0x00004000 );
        block.x |= ( ( endPoint[1][0].g << 18 ) & 0x00800000 );
        block.x |= ( ( endPoint[1][1].b << 13 ) & 0x00002000 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000003E);
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00000F80);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.y |= ( ( endPoint[1][1].g >> 4 ) & 0x00000002 ) | ( ( endPoint[1][1].g << 4 ) & 0x00000100 ) | ( ( endPoint[1][1].b >> 2 ) & 0x00000004 );
        block.y |= ( endPoint[1][1].b << 27 ) & 0x10000000;
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
        block.z |= ( ( endPoint[1][1].b << 9 ) & 0x00001000 ) | ( ( endPoint[1][1].b << 4 ) & 0x00000040 );
    }
    else if ( mode_type == candidateModeFlag[8])
    {
        block.x = candidateModeMemory[8];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x00001FE0 ) | ( ( endPoint[0][0].g << 15 ) & 0x007F8000 ) | ( ( endPoint[0][0].b << 25 ) & 0xFE000000 );
        block.y |= ( endPoint[0][0].b >> 7 ) & 0x00000001;
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000000F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0003E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x1F800000 );
        block.x |= ( ( endPoint[1][0].g << 20 ) & 0x01000000 ) | ( ( endPoint[1][0].b << 10 ) & 0x00004000 );
        block.x |= ( ( endPoint[1][0].b << 18 ) & 0x00800000 );
        block.x |= ( endPoint[1][1].b << 12 ) & 0x00002000;
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.y |= ( ( endPoint[1][1].g << 4 ) & 0x00000100 ) | ( ( endPoint[1][1].b >> 4 ) & 0x00000002 ) | ( ( endPoint[1][1].b >> 2 ) & 0x00000004 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000003E);
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00000F80);
        block.y |= ( endPoint[1][1].b << 18 ) & 0x00040000;
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
        block.z |= ( ( endPoint[1][1].b << 9 ) & 0x00001000 ) | ( ( endPoint[1][1].b << 4 ) & 0x00000040 );
    }
    else if ( mode_type == candidateModeFlag[9])
    {
        block.x = candidateModeMemory[9];
        block.x |= ( ( endPoint[0][0].r << 5 ) & 0x000007E0 ) | ( ( endPoint[0][0].g << 15 ) & 0x001F8000 ) | ( ( endPoint[0][0].b << 25 ) & 0x7E000000 );
        block.y |= ( ( endPoint[0][1].r << 3 ) & 0x000001F8 ) | ( ( endPoint[0][1].g << 13 ) & 0x0007E000 ) | ( ( endPoint[0][1].b << 23 ) & 0x1F800000 );
        block.x |= ( ( endPoint[1][0].g << 16 ) & 0x00200000 ) | ( ( endPoint[1][0].g << 20 ) & 0x01000000 );
        block.x |= ( ( endPoint[1][0].b << 17 ) & 0x00400000 ) | ( ( endPoint[1][0].b << 10 ) & 0x00004000 );
        block.x |= ( ( endPoint[1][1].b << 21 ) & 0x00800000 ) | ( ( endPoint[1][1].b << 12 ) & 0x00003000 );
        block.x |= ( ( endPoint[1][1].g << 26 ) & 0x80000000 ) | ( ( endPoint[1][1].g << 7 ) & 0x00000800 );
        block.yz |= ( endPoint[1][0].gr << uint2(9, 1) ) & uint2(0x00001E00, 0x0000007E);
        block.yz |= ( endPoint[1][1].gr << uint2(19, 7) ) & uint2(0x00780000, 0x00001F80);
        block.y |= ( endPoint[1][0].b << 29 ) & 0xE0000000;
        block.y |= ( ( endPoint[1][1].b >> 4 ) & 0x00000002 ) | ( ( endPoint[1][1].b >> 2 ) & 0x00000004 ) | ( ( endPoint[1][1].b >> 3 ) & 0x00000001 );
        block.z |= ( endPoint[1][0].b >> 3 ) & 0x00000001;
    }
}
void block_package( inout uint4 block, int2x3 endPoint, uint mode_type )
{
    block.x = ( ( endPoint[0].r << 5 ) & 0x00007FE0 ) | ( ( endPoint[0].g << 15 ) & 0x01FF8000 ) | ( ( endPoint[0].b << 25 ) & 0xFE000000 );
    block.y |= ( endPoint[0].b >> 7 ) & 0x00000007;

    if ( mode_type == candidateModeFlag[10])
    {
        block.x |= candidateModeMemory[10];
        block.y |= ( ( endPoint[1].r << 3 ) & 0x00001FF8 ) | ( ( endPoint[1].g << 13 ) & 0x007FE000 ) | ( ( endPoint[1].b << 23 ) & 0xFF800000 );
        block.z |= ( endPoint[1].b >> 9 ) & 0x00000001;
    }
    else if (mode_type == candidateModeFlag[11])
    {
        block.x |= candidateModeMemory[11];
        block.y |= ( ( endPoint[0].r << 2 ) & 0x00001000 ) | ( ( endPoint[0].g << 12 ) & 0x00400000 );
        block.y |= ( ( endPoint[1].r << 3 ) & 0x00000FF8 ) | ( ( endPoint[1].g << 13 ) & 0x003FE000 ) | ( ( endPoint[1].b << 23 ) & 0xFF800000 );
        block.z |= ( endPoint[0].b >> 10 ) & 0x00000001;
    }
    else if (mode_type == candidateModeFlag[12])// violate the spec in  [0].low
    {
        block.x |= candidateModeMemory[12];
        block.y |= ( ( endPoint[0].r << 2 ) & 0x00001000 ) | ( ( endPoint[0].g << 12 ) & 0x00400000 );
        block.y |= ( ( endPoint[0].r << 0 ) & 0x00000800 ) | ( ( endPoint[0].g << 10 ) & 0x00200000 );
        block.y |= ( endPoint[0].b << 20 ) & 0x80000000;
        block.y |= ( ( endPoint[1].r << 3 ) & 0x000007F8 ) | ( ( endPoint[1].g << 13 ) & 0x001FE000 ) | ( ( endPoint[1].b << 23 ) & 0x7F800000 );
        block.z |= ( endPoint[0].b >> 10 ) & 0x00000001;
    }
    else if (mode_type == candidateModeFlag[13])
    {
        block.x |= candidateModeMemory[13];
        block.y |= ( ( endPoint[0].r >> 3 ) & 0x00001F80 ) | ( ( endPoint[0].g << 7 ) & 0x007E0000 ) | ( ( endPoint[0].b << 17 ) & 0xF8000000 );
        block.y |= ( ( endPoint[1].r << 3 ) & 0x00000078 ) | ( ( endPoint[1].g << 13 ) & 0x0001E000 ) | ( ( endPoint[1].b << 23 ) & 0x07800000 );
        block.z |= ( endPoint[0].b >> 15 ) & 0x00000001;
    }
}
