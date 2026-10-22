// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: loadInput
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: Skinning10.fx
//
// The effect file for the Skinning10 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#ifndef SEPERABLE_BLUR_KERNEL_SIZE
#define SEPERABLE_BLUR_KERNEL_SIZE 3
#endif

static const int BLUR_KERNEL_BEGIN = SEPERABLE_BLUR_KERNEL_SIZE / -2; 
static const int BLUR_KERNEL_END = SEPERABLE_BLUR_KERNEL_SIZE / 2 + 1;
static const float FLOAT_BLUR_KERNEL_SIZE = (float)SEPERABLE_BLUR_KERNEL_SIZE;

cbuffer cbblurVS : register( b2)
{
	int2		g_iWidthHeight			: packoffset( c0 );
	int		    g_iKernelStart  		: packoffset( c0.z );
	int		    g_iKernelEnd	        : packoffset( c0.w );
};

//--------------------------------------------------------------------------------------
// defines
//--------------------------------------------------------------------------------------

Texture2DArray g_txShadow : register( t5 );
SamplerState g_samShadow : register( s5 );

//--------------------------------------------------------------------------------------
// Input/Output structures
//--------------------------------------------------------------------------------------

struct PSIn
{
    float4      Pos	    : SV_Position;		//Position
    float2      Tex	    : TEXCOORD;		    //Texture coordinate
    float2      ITex    : TEXCOORD2;
};

struct VSIn
{
    uint Pos	: SV_VertexID ;
};


PSIn main(VSIn inn)
{
    PSIn output;

    output.Pos.y  = -1.0f + (inn.Pos%2) * 2.0f ;
    output.Pos.x  = -1.0f + (inn.Pos/2) * 2.0f;
    output.Pos.z = .5;
    output.Pos.w = 1;
    output.Tex.x = inn.Pos/2;
    output.Tex.y = 1.0f - inn.Pos%2;
    output.ITex.x = (float)(g_iWidthHeight.x * output.Tex.x);
    output.ITex.y = (float)(g_iWidthHeight.y * output.Tex.y);
    return output;
}

