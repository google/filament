// RUN: %dxc -E main -T cs_6_0 -HV 2018 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: bufferStore

//--------------------------------------------------------------------------------------
// File: BC6HDecode.hlsl
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

cbuffer cbCS : register( b0 )
{
    uint g_tex_width;
    uint g_num_block_x;
    uint g_format;
    uint g_tex_size;
    uint g_start_block_id;
};

struct Mode
{
    uint type;
    bool transformed;
    uint4 prec;
};

static const uint candidateModeFlag[14] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
static const uint4 candidateModePrec[14] = { uint4(10,5,5,5), uint4(7,6,6,6),
    uint4(11,5,4,4), uint4(11,4,5,4), uint4(11,4,4,5), uint4(9,5,5,5),
    uint4(8,6,5,5), uint4(8,5,6,5), uint4(8,5,5,6), uint4(6,6,6,6),
    uint4(10,10,10,10), uint4(11,9,9,9), uint4(12,8,8,8), uint4(16,4,4,4) };

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

int extract_mode_index( uint4 block );
void extract_compressed_endpoints10( out int3 endPoint, uint mode_type, uint4 block );
void extract_compressed_endpoints11( out int3 endPoint, uint mode_type, uint4 block );
void SIGN_EXTEND( uint3 prec, inout int3 color );
uint extract_index_ONE( uint x, uint y, uint4 block );
void extract_compressed_endpoints20( out int3 endPoint, uint mode_type, uint4 block );
void extract_compressed_endpoints21( out int3 endPoint, uint mode_type, uint4 block );
void extract_compressed_endpoints22( out int3 endPoint, uint mode_type, uint4 block );
void extract_compressed_endpoints23( out int3 endPoint, uint mode_type, uint4 block );
uint extract_index_TWO( uint x, uint y, uint partition_index, uint4 block );
void unquantize( inout int3 color, uint prec );
void generate_palette_unquantized8( out uint3 palette, int3 low, int3 high, uint prec, int i );
void generate_palette_unquantized16( out uint3 palette, int3 low, int3 high, uint prec, int i );
uint3 finish_unquantize( int3 color );

StructuredBuffer<uint4> g_InBuff : register( t0 );
RWStructuredBuffer<uint4> g_OutBuff : register( u0 );

#define THREAD_GROUP_SIZE	64
#define BLOCK_SIZE_Y		4
#define BLOCK_SIZE_X		4
#define BLOCK_SIZE			(BLOCK_SIZE_Y * BLOCK_SIZE_X)
#define BLOCK_IN_GROUP		(THREAD_GROUP_SIZE / BLOCK_SIZE)

groupshared int4 shared_temp[THREAD_GROUP_SIZE];

[numthreads( THREAD_GROUP_SIZE, 1, 1 )]
void main( uint3 groupID : SV_GroupID, uint GI : SV_GroupIndex )
{
    uint blockInGroup = GI / BLOCK_SIZE;
    uint blockID = g_start_block_id + groupID.x * BLOCK_IN_GROUP + blockInGroup;
    uint threadInBlock = GI - blockInGroup * BLOCK_SIZE;
    
    if (4 == threadInBlock)
    {
        shared_temp[GI] = asint(g_InBuff[blockID]);
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    uint4 bc_data = asuint(shared_temp[blockInGroup * BLOCK_SIZE + 4]);
    int mode_index = extract_mode_index(bc_data);
    Mode mode;
    mode.type = mode_index + 1;
    mode.prec = candidateModePrec[mode_index];
    mode.transformed = (9 == mode_index) || (10 == mode_index) ? false : true;

    int3 ep = 0;
    if (0 == threadInBlock)
    {
        if ( mode.type > 10 )
        {
            extract_compressed_endpoints10( ep, mode.type, bc_data );
        }
        else
        {
            extract_compressed_endpoints20( ep, mode.type, bc_data );
        }
        if ( g_format == SIGNED_F16 )
            SIGN_EXTEND( mode.prec.x, ep );

        shared_temp[GI] = ep.xyzz;
    }
    else if (threadInBlock < 4)
    {
        if (1 == threadInBlock)
        {
            if ( mode.type > 10 )
            {
                extract_compressed_endpoints11( ep, mode.type, bc_data );
            }
            else
            {
                extract_compressed_endpoints21( ep, mode.type, bc_data );
            }
        }
        else
        {
            if ( mode.type <= 10 )
            {
                if (2 == threadInBlock)
                {
                    extract_compressed_endpoints22( ep, mode.type, bc_data );
                }
                else
                {
                    extract_compressed_endpoints23( ep, mode.type, bc_data );
                }
            }
        }
    }
#ifdef REF_DEVICE
    GroupMemoryBarrierWithGroupSync();
#endif

    if (threadInBlock < 4)
    {
        if ((1 == threadInBlock) || ((threadInBlock > 1) && (mode.type <= 10)))
        {
            if ( mode.transformed || g_format == SIGNED_F16 )
                SIGN_EXTEND( mode.prec.yzw, ep );

            if (mode.transformed)
            {
                ep += shared_temp[blockInGroup * BLOCK_SIZE + 0];
            }
        }

        unquantize( ep, mode.prec.x );
        shared_temp[GI] = ep.xyzz;
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
        int weight;
        uint3 palette;
        uint ep_index = blockInGroup * BLOCK_SIZE;
        if ( mode.type > 10 )
        {
            static const int aWeight4[] = {0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};
            uint index = extract_index_ONE( x, y, bc_data );
            weight = aWeight4[index];
        }
        else
        {
            uint partition_index = ( bc_data.z & 0x0003E000 ) >> 13;
            
            uint bit = (candidateSectionBit[partition_index] >> (y * 4 + x)) & 1;
            ep_index += bit * 2;
            
            static const int aWeight3[] = {0, 9, 18, 27, 37, 46, 55, 64};
            uint index = extract_index_TWO( x, y, partition_index, bc_data );
            weight = aWeight3[index];
        }
        int3 low = shared_temp[ep_index + 0].xyz;
        int3 high = shared_temp[ep_index + 1].xyz;
        palette = finish_unquantize(((low << 6) + (high - low) * weight + 32 ) >> 6);
        
        g_OutBuff[addr] = uint4( palette, 0x3C00 );
    }
}

static const uint candidateModeMask[2] = { 0x03, 0x1f };
static const uint candidateModeMemory[14] = { 0x00, 0x01,
    0x02, 0x06, 0x0A, 0x0E, 0x12, 0x16, 0x1A, 0x1E, 0x03, 0x07, 0x0B, 0x0F };
int extract_mode_index( uint4 block )
{
    int mode_index;
    
    uint type = block.r & candidateModeMask[0];
    if ( type == candidateModeMemory[0] )
    {
        mode_index = 0;
    }
    else if ( type == candidateModeMemory[1] )
    {
        mode_index = 1;
    }
    else
    {
        type = block.r & candidateModeMask[1];
        if ( type == candidateModeMemory[2] )
        {
            mode_index = 2;
        }
        else if ( type == candidateModeMemory[3] )
        {
            mode_index = 3;
        }
        else if ( type == candidateModeMemory[4] )
        {
            mode_index = 4;
        }
        else if ( type == candidateModeMemory[5] )
        {
            mode_index = 5;
        }
        else if ( type == candidateModeMemory[6] )
        {
            mode_index = 6;
        }
        else if ( type == candidateModeMemory[7] )
        {
            mode_index = 7;
        }
        else if ( type == candidateModeMemory[8] )
        {
            mode_index = 8;
        }
        else if ( type == candidateModeMemory[9] )
        {
            mode_index = 9;
        }
        else if ( type == candidateModeMemory[10] )
        {
            mode_index = 10;
        }
        else if ( type == candidateModeMemory[11] )
        {
            mode_index = 11;
        }
        else if ( type == candidateModeMemory[12] )
        {
            mode_index = 12;
        }
        else if ( type == candidateModeMemory[13] )
        {
            mode_index = 13;
        }
    }
    return mode_index;
}

void SIGN_EXTEND( uint3 prec, inout int3 color )
{
    uint3 p = 1 << (prec - 1);
    color = (color & p) ? (color & (p - 1)) - p : color;
}

void sign_extend( Mode mode, inout int2x3 endPoint[1] )
{
    if ( g_format == SIGNED_F16 )
        SIGN_EXTEND( mode.prec.x, endPoint[0][0] );
    if ( mode.transformed || g_format == SIGNED_F16 )
        SIGN_EXTEND( mode.prec.yzw, endPoint[0][1] );
}

void sign_extend( Mode mode, inout int2x3 endPoint[2] )
{
    if ( g_format == SIGNED_F16 )
        SIGN_EXTEND( mode.prec.x, endPoint[0][0] );
    if ( mode.transformed || g_format == SIGNED_F16 )
    {
        SIGN_EXTEND( mode.prec.yzw, endPoint[0][1] );
        SIGN_EXTEND( mode.prec.yzw, endPoint[1][0] );
        SIGN_EXTEND( mode.prec.yzw, endPoint[1][1] );
    }
}

void extract_compressed_endpoints( out int2x3 endPoint[1], uint mode_type, uint4 block )
{
    if ( mode_type == candidateModeFlag[10])
    {
        endPoint[0][0].r = ( block.x & 0x00007FE0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x01FF8000 ) >> 15;
        endPoint[0][0].b = ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x00001FF8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x007FE000 ) >> 13;
        endPoint[0][1].b = ( ( block.z & 0x00000001 ) << 9 ) | ( ( block.y & 0xFF800000 ) >> 23 );
    }
    else if (mode_type == candidateModeFlag[11])
    {
        endPoint[0][0].r = ( ( block.y & 0x00001000 ) >> 2 ) | ( ( block.x & 0x00007FE0 ) >> 5 );
        endPoint[0][0].g = ( ( block.y & 0x00400000 ) >> 12 ) | ( ( block.x & 0x01FF8000 ) >> 15 );
        endPoint[0][0].b = ( ( block.z & 0x00000001 ) << 10 ) | ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x00000FF8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x003FE000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0xFF800000 ) >> 23;
    }
    else if (mode_type == candidateModeFlag[12])// violate the spec in  [0][0]
    {
        endPoint[0][0].r = ( ( block.y & 0x00000800 ) >> 0 ) | ( ( block.y & 0x00001000 ) >> 2 ) | ( ( block.x & 0x00007FE0 ) >> 5 );
        endPoint[0][0].g = ( ( block.y & 0x00200000 ) >> 10 ) | ( ( block.y & 0x00400000 ) >> 12 ) | ( ( block.x & 0x01FF8000 ) >> 15 );
        endPoint[0][0].b = ( ( block.y & 0x80000000 ) >> 20 ) | ( ( block.z & 0x00000001 ) << 10) | ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x000007F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x001FE000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x7F800000 ) >> 23;
    }
    else if (mode_type == candidateModeFlag[13])
    {
        endPoint[0][0].r = ( ( block.y & 0x00001F80 ) << 3 ) | ( ( block.x & 0x00007FE0 ) >> 5 );
        endPoint[0][0].g = ( ( block.y & 0x007E0000 ) >> 7 ) | ( ( block.x & 0x01FF8000 ) >> 15 );
        endPoint[0][0].b = ( ( block.y & 0xF8000000 ) >> 17 ) | ( ( block.z & 0x00000001 ) << 15) | ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x00000078 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0001E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x07800000 ) >> 23;
    }
}

void extract_compressed_endpoints10( out int3 endPoint, uint mode_type, uint4 block )
{
    if ( mode_type == candidateModeFlag[10])
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.b |= ( ( block.y & 0x00000007 ) << 7 );
    }
    else if (mode_type == candidateModeFlag[11])
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.rg |= (block.y & uint2(0x00001000, 0x00400000)) >> uint2(2, 12);
        endPoint.b |= ( ( block.z & 0x00000001 ) << 10 ) | ( ( block.y & 0x00000007 ) << 7 );
    }
    else if (mode_type == candidateModeFlag[12])// violate the spec in  [0][0]
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25)
            | (block.y & uint3(0x00000800, 0x00200000, 0x80000000)) >> uint3(0, 10, 20);
        endPoint.rg |= (block.y & uint2(0x00001000, 0x00400000)) >> uint2(2, 12);
        endPoint.b |= ( ( block.z & 0x00000001 ) << 10) | ( ( block.y & 0x00000007 ) << 7 );
    }
    else if (mode_type == candidateModeFlag[13])
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.rb |= (block.y & uint2(0x00001F80, 0x00000007)) << uint2(3, 7);
        endPoint.gb |= (block.y & uint2(0x007E0000, 0xF8000000)) >> uint2(7, 17);
        endPoint.b |= (block.z & 0x00000001) << 15;
    }
}

void extract_compressed_endpoints11( out int3 endPoint, uint mode_type, uint4 block )
{
    if ( mode_type == candidateModeFlag[10])
    {
        endPoint = (block.y & uint3(0x00001FF8, 0x007FE000, 0xFF800000)) >> uint3(3, 13, 23);
        endPoint.b |= ( block.z & 0x00000001 ) << 9;
    }
    else if (mode_type == candidateModeFlag[11])
    {
        endPoint = (block.y & uint3(0x00000FF8, 0x003FE000, 0xFF800000)) >> uint3(3, 13, 23);
    }
    else if (mode_type == candidateModeFlag[12])// violate the spec in  [0][0]
    {
        endPoint = (block.y & uint3(0x000007F8, 0x001FE000, 0x7F800000)) >> uint3(3, 13, 23);
    }
    else if (mode_type == candidateModeFlag[13])
    {
        endPoint = (block.y & uint3(0x00000078, 0x0001E000, 0x07800000)) >> uint3(3, 13, 23);
    }
}

uint extract_index_ONE( uint x, uint y, uint4 block )
{
    if ( x == 0 && y == 0)
        return ( block.z >> 1) & 0x00000007;
    if ( y < 2 )
        return ( block.z >> ( y * 16 + x * 4 ) ) & 0x0000000F;
    return ( block.w >> ( ( y-2 ) * 16 + x * 4 ) ) & 0x0000000F;
}


//void extract_partition( out Partition partition, uint4 block )
//{
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

//	partition.index = ( block.z & 0x0003E000 ) >> 13;
//}

void extract_compressed_endpoints( out int2x3 endPoint[2], uint mode_type, uint4 block )
{
    if ( mode_type == candidateModeFlag[0])
    {
        endPoint[0][0].r = ( block.x & 0x00007FE0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x01FF8000 ) >> 15;
        endPoint[0][0].b = ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x000000F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0003E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x0F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000003E ) >> 1;
        endPoint[1][0].g = ( ( block.x & 0x00000004 ) << 2 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.x & 0x00000008 ) << 1 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00000F80 ) >> 7;
        endPoint[1][1].g = ( ( block.y & 0x00000100 ) >> 4 ) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.x & 0x00000010 ) >> 0 ) | ( ( block.z & 0x00001000 ) >> 9 ) | ( ( block.z & 0x00000040 ) >> 4 ) | ( ( block.y & 0x10000000 ) >> 27 ) | ( ( block.y & 0x00040000 ) >> 18 );
    }
    else if ( mode_type == candidateModeFlag[1])
    {
        endPoint[0][0].r = ( block.x & 0x00000FE0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x003F8000 ) >> 15;
        endPoint[0][0].b = ( block.x & 0xFE000000 ) >> 25;
        endPoint[0][1].r = ( block.y & 0x000001F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0007E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x1F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000007E ) >> 1;
        endPoint[1][0].g = ( ( block.x & 0x00000004 ) << 3 ) | ( ( block.x & 0x01000000 ) >> 20 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.x & 0x00400000 ) >> 17 ) | ( ( block.x & 0x00004000 ) >> 10 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00001F80 ) >> 7;
        endPoint[1][1].g = ( ( block.x & 0x00000018 ) << 1 ) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.y & 0x00000002 ) << 4 ) | ( ( block.y & 0x00000004 ) << 2 ) | ( ( block.y & 0x00000001 ) << 3 ) | ( ( block.x & 0x00800000 ) >> 21 ) | ( ( block.x & 0x00003000 ) >> 12 );
    }
    else if ( mode_type == candidateModeFlag[2])
    {
        endPoint[0][0].r = ( ( block.y & 0x00000100 ) << 2 ) | ( ( block.x & 0x00007FE0 ) >> 5 );// fixed a bug in v0.31
        endPoint[0][0].g = ( ( block.y & 0x00020000 ) >> 7 ) | ( ( block.x & 0x01FF8000 ) >> 15 );// fixed a bug in v0.31
        endPoint[0][0].b = ( ( block.y & 0x08000000 ) >> 17 ) | ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );// fixed a bug in v0.31
        endPoint[0][1].r = ( block.y & 0x000000F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0001E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x07800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000003E ) >> 1;
        endPoint[1][0].g = ( block.y & 0x00001E00 ) >> 9;
        endPoint[1][0].b = ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00000F80 ) >> 7;
        endPoint[1][1].g = ( block.y & 0x00780000 ) >> 19;
        endPoint[1][1].b = ( ( block.z & 0x00001000 ) >> 9 ) | ( ( block.z & 0x00000040 ) >> 4 ) | ( ( block.y & 0x10000000 ) >> 27 ) | ( ( block.y & 0x00040000 ) >> 18 );
    }
    else if ( mode_type == candidateModeFlag[3])
    {
        endPoint[0][0].r = ( ( block.y & 0x00000080 ) << 3 ) | ( ( block.x & 0x00007FE0 ) >> 5 );// fixed a bug in v0.31
        endPoint[0][0].g = ( ( block.y & 0x00040000 ) >> 8 ) | ( ( block.x & 0x01FF8000 ) >> 15 );// fixed a bug in v0.31
        endPoint[0][0].b = ( ( block.y & 0x08000000 ) >> 17 ) | ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );// fixed a bug in v0.31
        endPoint[0][1].r = ( block.y & 0x00000078 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0003E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x07800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000001E ) >> 1;
        endPoint[1][0].g = ( ( block.z & 0x00000800 ) >> 7 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00000780 ) >> 7;
        endPoint[1][1].g = ( ( block.y & 0x00000100 ) >> 4) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.z & 0x00001000 ) >> 9 ) | ( ( block.z & 0x00000040 ) >> 4 ) | ( ( block.y & 0x10000000 ) >> 27 ) | ( ( block.z & 0x00000020 ) >> 5 );
    }
    else if ( mode_type == candidateModeFlag[4])
    {
        endPoint[0][0].r = ( ( block.y & 0x00000080 ) << 3 ) | ( ( block.x & 0x00007FE0 ) >> 5 );// fixed a bug in v0.31
        endPoint[0][0].g = ( ( block.y & 0x00020000 ) >> 7 ) | ( ( block.x & 0x01FF8000 ) >> 15 );// fixed a bug in v0.31
        endPoint[0][0].b = ( ( block.y & 0x10000000 ) >> 18 ) | ( ( block.y & 0x00000007 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );// fixed a bug in v0.31
        endPoint[0][1].r = ( block.y & 0x00000078 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0001E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x0F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000001E ) >> 1;
        endPoint[1][0].g = ( block.y & 0x00001E00 ) >> 9;
        endPoint[1][0].b = ( ( block.y & 0x00000100 ) >> 4 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00000780 ) >> 7;
        endPoint[1][1].g = ( block.y & 0x00780000 ) >> 19;
        endPoint[1][1].b = ( ( block.z & 0x00000800 ) >> 7 ) | ( ( block.z & 0x00001000 ) >> 9 ) | ( ( block.z & 0x00000060 ) >> 4 ) | ( ( block.y & 0x00040000 ) >> 18 );
    }
    else if ( mode_type == candidateModeFlag[5])
    {
        endPoint[0][0].r = ( block.x & 0x00003FE0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x00FF8000 ) >> 15;
        endPoint[0][0].b = ( ( block.y & 0x00000003 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x000000F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0003E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x0F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000003E ) >> 1;
        endPoint[1][0].g = ( ( block.x & 0x01000000 ) >> 20 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.x & 0x00004000 ) >> 10 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00000F80 ) >> 7;
        endPoint[1][1].g = ( ( block.y & 0x00000100 ) >> 4 ) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.y & 0x00000004 ) << 2 ) | ( ( block.z & 0x00001000 ) >> 9 ) | ( ( block.z & 0x00000040 ) >> 4 ) | ( ( block.y & 0x10000000 ) >> 27 ) | ( ( block.y & 0x00040000 ) >> 18 );
    }
    else if ( mode_type == candidateModeFlag[6])
    {
        endPoint[0][0].r = ( block.x & 0x00001FE0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x007F8000 ) >> 15;
        endPoint[0][0].b = ( ( block.y & 0x00000001 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x000001F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0003E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x0F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000007E ) >> 1;
        endPoint[1][0].g = ( ( block.x & 0x01000000 ) >> 20 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.x & 0x00004000 ) >> 10 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00001F80 ) >> 7;
        endPoint[1][1].g = ( ( block.x & 0x00002000 ) >> 9 ) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.y & 0x00000006 ) << 2 ) | ( ( block.x & 0x00800000 ) >> 21 ) | ( ( block.y & 0x10000000 ) >> 27 ) | ( ( block.y & 0x00040000 ) >> 18 );
    }
    else if ( mode_type == candidateModeFlag[7])
    {
        endPoint[0][0].r = ( block.x & 0x00001FE0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x007F8000 ) >> 15;
        endPoint[0][0].b = ( ( block.y & 0x00000001 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x000000F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0007E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x0F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000003E ) >> 1;
        endPoint[1][0].g = ( ( block.x & 0x00800000 ) >> 18 ) | ( ( block.x & 0x01000000 ) >> 20 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.x & 0x00004000 ) >> 10 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00000F80 ) >> 7;
        endPoint[1][1].g = ( ( block.y & 0x00000002 ) << 4 ) | ( ( block.y & 0x00000100 ) >> 4 ) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.y & 0x00000004 ) << 2 ) | ( ( block.z & 0x00001000 ) >> 9 ) | ( ( block.z & 0x00000040 ) >> 4 ) | ( ( block.y & 0x10000000 ) >> 27 ) | ( ( block.x & 0x00002000 ) >> 13 );
    }
    else if ( mode_type == candidateModeFlag[8])
    {
        endPoint[0][0].r = ( block.x & 0x00001FE0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x007F8000 ) >> 15;
        endPoint[0][0].b = ( ( block.y & 0x00000001 ) << 7 ) | ( ( block.x & 0xFE000000 ) >> 25 );
        endPoint[0][1].r = ( block.y & 0x000000F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0003E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x1F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000003E ) >> 1;
        endPoint[1][0].g = ( ( block.x & 0x01000000 ) >> 20 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.x & 0x00800000 ) >> 18 ) | ( ( block.x & 0x00004000 ) >> 10 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00000F80 ) >> 7;
        endPoint[1][1].g = ( ( block.y & 0x00000100 ) >> 4 ) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.y & 0x00000002 ) << 4 ) | ( ( block.y & 0x00000004 ) << 2 ) | ( ( block.z & 0x00001000 ) >> 9 ) | ( ( block.z & 0x00000040 ) >> 4 ) | ( ( block.x & 0x00002000 ) >> 12 ) | ( ( block.y & 0x00040000 ) >> 18 );
    }
    else if ( mode_type == candidateModeFlag[9])
    {
        endPoint[0][0].r = ( block.x & 0x000007E0 ) >> 5;
        endPoint[0][0].g = ( block.x & 0x001F8000 ) >> 15;
        endPoint[0][0].b = ( block.x & 0x7E000000 ) >> 25;
        endPoint[0][1].r = ( block.y & 0x000001F8 ) >> 3;
        endPoint[0][1].g = ( block.y & 0x0007E000 ) >> 13;
        endPoint[0][1].b = ( block.y & 0x1F800000 ) >> 23;
        endPoint[1][0].r = ( block.z & 0x0000007E ) >> 1;
        endPoint[1][0].g = ( ( block.x & 0x00200000 ) >> 16 ) | ( ( block.x & 0x01000000 ) >> 20 ) | ( ( block.y & 0x00001E00 ) >> 9 );
        endPoint[1][0].b = ( ( block.x & 0x00400000 ) >> 17 ) | ( ( block.x & 0x00004000 ) >> 10 ) | ( ( block.z & 0x00000001 ) << 3 ) | ( ( block.y & 0xE0000000 ) >> 29 );
        endPoint[1][1].r = ( block.z & 0x00001F80 ) >> 7;
        endPoint[1][1].g = ( ( block.x & 0x80000000 ) >> 26 ) | ( ( block.x & 0x00000800 ) >> 7 ) | ( ( block.y & 0x00780000 ) >> 19 );
        endPoint[1][1].b = ( ( block.y & 0x00000002 ) << 4 ) | ( ( block.y & 0x00000004 ) << 2 ) | ( ( block.y & 0x00000001 ) << 3 ) | ( ( block.x & 0x00800000 ) >> 21 ) | ( ( block.x & 0x00003000 ) >> 12 );
    }	
}

void extract_compressed_endpoints20( out int3 endPoint, uint mode_type, uint4 block )
{
    if ( mode_type == candidateModeFlag[0])
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.b |= ( block.y & 0x00000007 ) << 7;
    }
    else if ( mode_type == candidateModeFlag[1])
    {
        endPoint = (block.x & uint3(0x00000FE0, 0x003F8000, 0xFE000000)) >> uint3(5, 15, 25);
    }
    else if ( mode_type == candidateModeFlag[2])
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.rb |= (block.y & uint2(0x00000100, 0x00000007)) << uint2(1, 7);
        endPoint.gb |= (block.y & uint2(0x00020000, 0x08000000)) >> uint2(8, 18);
    }
    else if ( mode_type == candidateModeFlag[3])
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.rb |= (block.y & uint2(0x00000080, 0x00000007)) << uint2(2, 7);
        endPoint.gb |= (block.y & uint2(0x00040000, 0x08000000)) >> uint2(9, 18);
    }
    else if ( mode_type == candidateModeFlag[4])
    {
        endPoint = (block.x & uint3(0x00007FE0, 0x01FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.rb |= (block.y & uint2(0x00000080, 0x00000007)) << uint2(2, 7);
        endPoint.gb |= (block.y & uint2(0x00020000, 0x10000000)) >> uint2(8, 19);
    }
    else if ( mode_type == candidateModeFlag[5])
    {
        endPoint = (block.x & uint3(0x00003FE0, 0x00FF8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.b |= ( block.y & 0x00000003 ) << 7;
    }
    else if ( mode_type == candidateModeFlag[6])
    {
        endPoint = (block.x & uint3(0x00001FE0, 0x007F8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.b |= ( block.y & 0x00000001 ) << 7;
    }
    else if ( mode_type == candidateModeFlag[7])
    {
        endPoint = (block.x & uint3(0x00001FE0, 0x007F8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.b |= ( block.y & 0x00000001 ) << 7;
    }
    else if ( mode_type == candidateModeFlag[8])
    {
        endPoint = (block.x & uint3(0x00001FE0, 0x007F8000, 0xFE000000)) >> uint3(5, 15, 25);
        endPoint.b |= ( block.y & 0x00000001 ) << 7;
    }
    else if ( mode_type == candidateModeFlag[9])
    {
        endPoint = (block.x & uint3(0x000007E0, 0x001F8000, 0x7E000000)) >> uint3(5, 15, 25);
    }
}

void extract_compressed_endpoints21( out int3 endPoint, uint mode_type, uint4 block )
{
    uint3 mask;
    if ( mode_type == candidateModeFlag[0])
    {
        mask = uint3(0x000000F8, 0x0003E000, 0x0F800000);
    }
    else if ( mode_type == candidateModeFlag[1])
    {
        mask = uint3(0x000001F8, 0x0007E000, 0x1F800000);
    }
    else if ( mode_type == candidateModeFlag[2])
    {
        mask = uint3(0x000000F8, 0x0001E000, 0x07800000);
    }
    else if ( mode_type == candidateModeFlag[3])
    {
        mask = uint3(0x00000078, 0x0003E000, 0x07800000);
    }
    else if ( mode_type == candidateModeFlag[4])
    {
        mask = uint3(0x00000078, 0x0001E000, 0x0F800000);
    }
    else if ( mode_type == candidateModeFlag[5])
    {
        mask = uint3(0x000000F8, 0x0003E000, 0x0F800000);
    }
    else if ( mode_type == candidateModeFlag[6])
    {
        mask = uint3(0x000001F8, 0x0003E000, 0x0F800000);
    }
    else if ( mode_type == candidateModeFlag[7])
    {
        mask = uint3(0x000000F8, 0x0007E000, 0x0F800000);
    }
    else if ( mode_type == candidateModeFlag[8])
    {
        mask = uint3(0x000000F8, 0x0003E000, 0x1F800000);
    }
    else //if ( mode_type == candidateModeFlag[9])
    {
        mask = uint3(0x000001F8, 0x0007E000, 0x1F800000);
    }
    endPoint = (block.y & mask) >> uint3(3, 13, 23);
}

void extract_compressed_endpoints22( out int3 endPoint, uint mode_type, uint4 block )
{
    if ( mode_type == candidateModeFlag[0])
    {
        endPoint = (block.zyy & uint3(0x0000003E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.gb |= (block.x & uint2(0x00000004, 0x00000008)) << uint2(2, 1);
        endPoint.b |= (block.z & 0x00000001) << 3;
    }
    else if ( mode_type == candidateModeFlag[1])
    {
        endPoint = (block.zyy & uint3(0x0000007E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.gb |= (block.x & uint2(0x01000000, 0x00004000)) >> uint2(20, 10)
                    | (block.xz & uint2(0x00000004, 0x00000001)) << 3;
        endPoint.b |= (block.x & 0x00400000) >> 17;
    }
    else if ( mode_type == candidateModeFlag[2])
    {
        endPoint = (block.zyy & uint3(0x0000003E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.b |= (block.z & 0x00000001) << 3;
    }
    else if ( mode_type == candidateModeFlag[3])
    {
        endPoint = (block.zzy & uint3(0x0000001E, 0x00000800, 0xE0000000)) >> uint3(1, 7, 29);
        endPoint.g |= (block.y & 0x00001E00) >> 9;
        endPoint.b |= (block.z & 0x00000001) << 3;
    }
    else if ( mode_type == candidateModeFlag[4])
    {
        endPoint = (block.zyy & uint3(0x0000001E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.b |= (block.y & 0x00000100) >> 4
                    | (block.z & 0x00000001) << 3;
    }
    else if ( mode_type == candidateModeFlag[5])
    {
        endPoint = (block.zyy & uint3(0x0000003E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.gb |= (block.x & uint2(0x01000000, 0x00004000)) >> uint2(20, 10);
        endPoint.b |= ( block.z & 0x00000001 ) << 3;
    }
    else if ( mode_type == candidateModeFlag[6])
    {
        endPoint = (block.zyy & uint3(0x0000007E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.gb |= (block.x & uint2(0x01000000, 0x00004000)) >> uint2(20, 10);
        endPoint.b |= ( block.z & 0x00000001 ) << 3;
    }
    else if ( mode_type == candidateModeFlag[7])
    {
        endPoint = (block.zyy & uint3(0x0000003E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.gb |= (block.x & uint2(0x01000000, 0x00004000)) >> uint2(20, 10);
        endPoint.g |= (block.x & 0x00800000) >> 18;
        endPoint.b |= (block.z & 0x00000001) << 3;
    }
    else if ( mode_type == candidateModeFlag[8])
    {
        endPoint = (block.zyy & uint3(0x0000003E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.gb |= (block.x & uint2(0x01000000, 0x00004000)) >> uint2(20, 10);
        endPoint.b |= (block.x & 0x00800000) >> 18
                    | ( block.z & 0x00000001 ) << 3;
    }
    else if ( mode_type == candidateModeFlag[9])
    {
        endPoint = (block.zyy & uint3(0x0000007E, 0x00001E00, 0xE0000000)) >> uint3(1, 9, 29);
        endPoint.gb |= (block.x & uint2(0x01000000, 0x00004000)) >> uint2(20, 10)
                    | (block.x & uint2(0x00200000, 0x00400000)) >> uint2(16, 17);
        endPoint.b |= ( block.z & 0x00000001 ) << 3;
    }	
}

void extract_compressed_endpoints23( out int3 endPoint, uint mode_type, uint4 block )
{
    if ( mode_type == candidateModeFlag[0])
    {
        endPoint = (block.zyz & uint3(0x00000F80, 0x00780000, 0x00001000)) >> uint3(7, 19, 9);
        endPoint.gb |= (block.y & uint2(0x00000100, 0x00040000)) >> uint2(4, 18);
        endPoint.b |= (block.x & 0x00000010) >> 0
                    | (block.y & 0x10000000) >> 27
                    | (block.z & 0x00000040) >> 4;
    }
    else if ( mode_type == candidateModeFlag[1])
    {
        endPoint = (block.zyx & uint3(0x00001F80, 0x00780000, 0x00800000)) >> uint3(7, 19, 21);
        endPoint.gb |= (block.xy & uint2(0x00000018, 0x00000002)) << uint2(1, 4);
        endPoint.b |= (block.y & 0x00000004) << 2
                    | (block.y & 0x00000001) << 3
                    | (block.x & 0x00003000) >> 12;
    }
    else if ( mode_type == candidateModeFlag[2])
    {
        endPoint = (block.zyz & uint3(0x00000F80, 0x00780000, 0x00001000)) >> uint3(7, 19, 9);
        endPoint.b |= (block.y & 0x00040000) >> 18
                    | (block.z & 0x00000040) >> 4
                    | (block.y & 0x10000000) >> 27;
    }
    else if ( mode_type == candidateModeFlag[3])
    {
        endPoint = (block.zyz & uint3(0x00000780, 0x00780000, 0x00001000)) >> uint3(7, 19, 9);
        endPoint.gb |= (block.yz & uint2(0x00000100, 0x00000040)) >> 4;
        endPoint.b |= (block.z & 0x00000020) >> 5
                    | (block.y & 0x10000000) >> 27;
    }
    else if ( mode_type == candidateModeFlag[4])
    {
        endPoint = (block.zyz & uint3(0x00000780, 0x00780000, 0x00001000)) >> uint3(7, 19, 9);
        endPoint.b |= (block.y & 0x00040000) >> 18
                    | (block.z & 0x00000800) >> 7
                    | (block.z & 0x00000060) >> 4;
    }
    else if ( mode_type == candidateModeFlag[5])
    {
        endPoint = (block.zyz & uint3(0x00000F80, 0x00780000, 0x00001000)) >> uint3(7, 19, 9);
        endPoint.gb |= (block.y & uint2(0x00000100, 0x10000000)) >> uint2(4, 27);
        endPoint.b |= (block.y & 0x00040000) >> 18
                    | (block.y & 0x00000004) << 2
                    | (block.z & 0x00000040) >> 4;
    }
    else if ( mode_type == candidateModeFlag[6])
    {
        endPoint = (block.zyx & uint3(0x00001F80, 0x00780000, 0x00800000)) >> uint3(7, 19, 21);
        endPoint.gb |= (block.xy & uint2(0x00002000, 0x00040000)) >> uint2(9, 18);
        endPoint.b |= (block.y & 0x10000000) >> 27
                    | (block.y & 0x00000006) << 2;
    }
    else if ( mode_type == candidateModeFlag[7])
    {
        endPoint = (block.zyz & uint3(0x00000F80, 0x00780000, 0x00001000)) >> uint3(7, 19, 9);
        endPoint.gb |= (block.y & uint2(0x00000002, 0x00000004)) << uint2(4, 2)
                    | (block.yz & uint2(0x00000100, 0x00000040)) >> 4;
        endPoint.b |= (block.y & 0x10000000) >> 27
                    | (block.x & 0x00002000) >> 13;
    }
    else if ( mode_type == candidateModeFlag[8])
    {
        endPoint = (block.zyz & uint3(0x00000F80, 0x00780000, 0x00001000)) >> uint3(7, 19, 9);
        endPoint.gb |= (block.y & uint2(0x00000100, 0x00040000)) >> uint2(4, 18);
        endPoint.b |= (block.z & 0x00000040) >> 4
                    | (block.y & 0x00000002) << 4
                    | (block.y & 0x00000004) << 2
                    | (block.x & 0x00002000) >> 12;
    }
    else if ( mode_type == candidateModeFlag[9])
    {
        endPoint = (block.zyx & uint3(0x00001F80, 0x00780000, 0x00800000)) >> uint3(7, 19, 21);
        endPoint.gb |= (block.x & uint2(0x00000800, 0x00003000)) >> uint2(7, 12);
        endPoint.g |= (block.x & 0x80000000) >> 26;
        endPoint.b |= (block.y & 0x00000002) << 4
                    | (block.y & 0x00000004) << 2
                    | (block.y & 0x00000001) << 3;
    }	
}

/*const uint2 candidateFixUpIndex[32] = 
{
    {3,3},{3,3},{3,3},{3,3},
    {3,3},{3,3},{3,3},{3,3},
    {3,3},{3,3},{3,3},{3,3},
    {3,3},{3,3},{3,3},{3,3},
    {3,3},{2,0},{0,2},{2,0},
    {2,0},{0,2},{0,2},{3,3},
    {2,0},{0,2},{2,0},{2,0},
    {0,2},{0,2},{2,0},{2,0}
};*/
uint extract_index_TWO( uint x, uint y, uint partition_index, uint4 block )
{
    static const uint candidateFixUpIndex1D[32] = 
    {
        15,15,15,15,
        15,15,15,15,
        15,15,15,15,
        15,15,15,15,
        15,2,8,2,
        2,8,8,15,
        2,8,2,2,
        8,8,2,2
    };
    
    if ( x == 0 && y == 0 )
        return ( block.z >> 18 ) & 0x00000003;
    uint index = y * 4 + x;
    if ( index < candidateFixUpIndex1D[partition_index] )
    {
        if ( index < 5 )
            return ( block.z >> ( index * 3 + 17 ) ) & 0x00000007;
        return ( block.w >> ( index * 3 - 15 ) ) & 0x00000007;
    }
    if ( index == candidateFixUpIndex1D[partition_index] )
    {
        if ( index < 5 )
            return ( block.z >> ( index * 3 + 17 ) ) & 0x00000003;
        return ( block.w >> ( index * 3 - 15 ) ) & 0x00000003;
    }
    if ( index < 5 )
        return ( block.z >> ( index * 3 + 16 ) ) & 0x00000007;
    if ( index > 5 )
        return ( block.w >> ( index * 3 - 16 ) ) & 0x00000007;
    return ( ( block.z >> 31 ) & 0x00000001 ) | ( ( block.w << 1 ) & 0x00000006 );
}

void unquantize( inout int3 color, uint prec )
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
            uint3 s = color >= 0 ? 0 : 1;
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
        color = ( color < 0 ) ? (( -color ) | 0x8000) : color;
    }
    return asuint(color);
}

void generate_palette_unquantized8( out uint3 palette, int3 low, int3 high, uint prec, int i )
{
    static const int aWeight3[] = {0, 9, 18, 27, 37, 46, 55, 64};
    
    int3 tmp = ( low * ( 64 - aWeight3[i] ) + high * aWeight3[i] + 32 ) >> 6;
    palette = finish_unquantize( tmp );
}

void generate_palette_unquantized16( out uint3 palette, int3 low, int3 high, uint prec, int i )
{
    static const int aWeight4[] = {0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};
    
    int3 tmp = ( low * ( 64 - aWeight4[i] ) + high * aWeight4[i] + 32 ) >> 6;
    palette = finish_unquantize( tmp );
}
