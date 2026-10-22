// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: dx.op.sample
// CHECK: dx.op.sample
// CHECK: dx.op.sample

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



//float PSDepth

//------------------------------------------------------------------------------
// Logarithmic filtering
//------------------------------------------------------------------------------

float log_conv ( float x0, float X, float y0, float Y )
{
    return (X + log(x0 + (y0 * exp(Y - X))));
}


//--------------------------------------------------------------------------------------
// Pixel shader that performs bump mapping on the final vertex
//--------------------------------------------------------------------------------------
float2 main(PSIn input) : SV_Target
{	
/*
	float2 centerDistance;
	if ( input.Tex.x  < .5 ) centerDistance.x = (1.0 - input.Tex.x);
	else centerDistance.x = input.Tex.x;
	if ( input.Tex.y  < .5 ) centerDistance.y = (1.0 - input.Tex.y);
	else centerDistance.y = input.Tex.y;
	if (centerDistance.x < centerDistance.y) centerDistance.x = centerDistance.y;
	centerDistance.x -= .2;
	centerDistance.x *= (1.0f / .8);
	
    float store_samples[8];
    int ind = 0;
    for (int y = g_iKernelStart; y < g_iKernelEnd; ++y) {
        store_samples[ind] = g_txShadow.Load( int3(input.ITex.x, input.ITex.y+(float)y * centerDistance.x, 0) ).r;
    }
    const float c = (1.f/5.f);    
    
    float accum;
    accum = log_conv( c, store_samples[0], c, store_samples[1] );    
    
    ind = 0;
    for (y = g_iKernelStart; y < g_iKernelEnd; ++y) {
        ind++;
        accum += log_conv( 1.0f, accum, c, store_samples[ind] );
    }
    float2 rt;
    rt.x = accum;
    return rt;
    */
    
    
    /*    
    float2 dep = 0;

	float2 centerDistance;
	if ( input.Tex.x  < .5 ) centerDistance.x = (1.0 - input.Tex.x);
	else centerDistance.x = input.Tex.x;
	if ( input.Tex.y  < .5 ) centerDistance.y = (1.0 - input.Tex.y);
	else centerDistance.y = input.Tex.y;
	if (centerDistance.x < centerDistance.y) centerDistance.x = centerDistance.y;
	centerDistance.x -= 0;
	centerDistance.x *= (1.0f / 1.0f);
	
	if (centerDistance.x < centerDistance.y) centerDistance.x = centerDistance.y;
    for (int y = g_iKernelStart; y < g_iKernelEnd; ++y) {
        dep += g_txShadow.Load( int3(input.ITex.x, input.ITex.y+(float)y * centerDistance.x, 0) ).rg;
    }
    
    
    dep /= (g_iKernelEnd - g_iKernelStart);
    return dep;
    
    */
    
    
    float2 dep=0;
    [unroll]for ( int y = BLUR_KERNEL_BEGIN; y < BLUR_KERNEL_END; ++y ) {
        dep += g_txShadow.Sample( g_samShadow,  float3( input.Tex.x, input.Tex.y, 0 ), int2( 0,y ) ).rg;
    }
    dep /= FLOAT_BLUR_KERNEL_SIZE;
    return dep;  
    
    //return g_txShadow.Sample(g_samShadow,  float3(input.Tex.x, input.Tex.y, 0) ).rg;
}



