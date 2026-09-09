//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#define RootSig \
	"RootFlags(0), " \
	"RootConstants(b0, num32BitConstants = 4), " \
	"DescriptorTable(SRV(t0, numDescriptors = 1))," \
	"DescriptorTable(UAV(u0, numDescriptors = 4))," \
	"StaticSampler(s0," \
		"addressU = TEXTURE_ADDRESS_CLAMP," \
		"addressV = TEXTURE_ADDRESS_CLAMP," \
		"addressW = TEXTURE_ADDRESS_CLAMP," \
		"filter = FILTER_MIN_MAG_MIP_LINEAR)"

#ifndef NON_POWER_OF_TWO
#define NON_POWER_OF_TWO 0
#endif

RWTexture2D<float4> OutMip1 : register(u0);
RWTexture2D<float4> OutMip2 : register(u1);
RWTexture2D<float4> OutMip3 : register(u2);
RWTexture2D<float4> OutMip4 : register(u3);
Texture2D<float4> SrcMip : register(t0);
SamplerState BilinearClamp : register(s0);

cbuffer CB : register(b0)
{
	uint SrcMipLevel;	// Texture level of source mip
	uint NumMipLevels;	// Number of OutMips to write: [1, 4]
	float2 TexelSize;	// 1.0 / OutMip1.Dimensions
}

// The reason for separating channels is to reduce bank conflicts in the
// local data memory controller.  A large stride will cause more threads
// to collide on the same memory bank.
groupshared float gs_R[64];
groupshared float gs_G[64];
groupshared float gs_B[64];
groupshared float gs_A[64];

void StoreColor( uint Index, float4 Color )
{
	gs_R[Index] = Color.r;
	gs_G[Index] = Color.g;
	gs_B[Index] = Color.b;
	gs_A[Index] = Color.a;
}

float4 LoadColor( uint Index )
{
	return float4( gs_R[Index], gs_G[Index], gs_B[Index], gs_A[Index]);
}

float3 LinearToSRGB(float3 x)
{
	// This is exactly the sRGB curve
	//return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
	 
	// This is cheaper but nearly equivalent
	return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

float4 PackColor(float4 Linear)
{
#ifdef CONVERT_TO_SRGB
	return float4(LinearToSRGB(Linear.rgb), Linear.a);
#else
	return Linear;
#endif
}

[RootSignature(RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
	// One bilinear sample is insufficient when scaling down by more than 2x.
	// You will slightly undersample in the case where the source dimension
	// is odd.  This is why it's a really good idea to only generate mips on
	// power-of-two sized textures.  Trying to handle the undersampling case
	// will force this shader to be slower and more complicated as it will
	// have to take more source texture samples.
#if NON_POWER_OF_TWO == 0
	float2 UV = TexelSize * (DTid.xy + 0.5);
	float4 Src1 = SrcMip.SampleLevel(BilinearClamp, UV, SrcMipLevel);
#elif NON_POWER_OF_TWO == 1
	// > 2:1 in X dimension
	// Use 2 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
	// horizontally.
	float2 UV1 = TexelSize * (DTid.xy + float2(0.25, 0.5));
	float2 Off = TexelSize * float2(0.5, 0.0);
	float4 Src1 = 0.5 * (SrcMip.SampleLevel(BilinearClamp, UV1, SrcMipLevel) +
		SrcMip.SampleLevel(BilinearClamp, UV1 + Off, SrcMipLevel));
#elif NON_POWER_OF_TWO == 2
	// > 2:1 in Y dimension
	// Use 2 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
	// vertically.
	float2 UV1 = TexelSize * (DTid.xy + float2(0.5, 0.25));
	float2 Off = TexelSize * float2(0.0, 0.5);
	float4 Src1 = 0.5 * (SrcMip.SampleLevel(BilinearClamp, UV1, SrcMipLevel) +
		SrcMip.SampleLevel(BilinearClamp, UV1 + Off, SrcMipLevel));
#elif NON_POWER_OF_TWO == 3
	// > 2:1 in in both dimensions
	// Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
	// in both directions.
	float2 UV1 = TexelSize * (DTid.xy + float2(0.25, 0.25));
	float2 O = TexelSize * 0.5;
	float4 Src1 = SrcMip.SampleLevel(BilinearClamp, UV1, SrcMipLevel);
	Src1 += SrcMip.SampleLevel(BilinearClamp, UV1 + float2(O.x, 0.0), SrcMipLevel);
	Src1 += SrcMip.SampleLevel(BilinearClamp, UV1 + float2(0.0, O.y), SrcMipLevel);
	Src1 += SrcMip.SampleLevel(BilinearClamp, UV1 + float2(O.x, O.y), SrcMipLevel);
	Src1 *= 0.25;
#endif

	OutMip1[DTid.xy] = PackColor(Src1);

	// A scalar (constant) branch can exit all threads coherently.
	if (NumMipLevels == 1)
		return;

	// Without lane swizzle operations, the only way to share data with other
	// threads is through LDS.
	StoreColor(GI, Src1);

	// This guarantees all LDS writes are complete and that all threads have
	// executed all instructions so far (and therefore have issued their LDS
	// write instructions.)
	GroupMemoryBarrierWithGroupSync();

	// With low three bits for X and high three bits for Y, this bit mask
	// (binary: 001001) checks that X and Y are even.
	if ((GI & 0x9) == 0)
	{
		float4 Src2 = LoadColor(GI + 0x01);
		float4 Src3 = LoadColor(GI + 0x08);
		float4 Src4 = LoadColor(GI + 0x09);
		Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

		OutMip2[DTid.xy / 2] = PackColor(Src1);
		StoreColor(GI, Src1);
	}

	if (NumMipLevels == 2)
		return;

	GroupMemoryBarrierWithGroupSync();

	// This bit mask (binary: 011011) checks that X and Y are multiples of four.
	if ((GI & 0x1B) == 0)
	{
		float4 Src2 = LoadColor(GI + 0x02);
		float4 Src3 = LoadColor(GI + 0x10);
		float4 Src4 = LoadColor(GI + 0x12);
		Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

		OutMip3[DTid.xy / 4] = PackColor(Src1);
		StoreColor(GI, Src1);
	}

	if (NumMipLevels == 3)
		return;

	GroupMemoryBarrierWithGroupSync();

	// This bit mask would be 111111 (X & Y multiples of 8), but only one
	// thread fits that criteria.
	if (GI == 0)
	{
		float4 Src2 = LoadColor(GI + 0x04);
		float4 Src3 = LoadColor(GI + 0x20);
		float4 Src4 = LoadColor(GI + 0x24);
		Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);

		OutMip4[DTid.xy / 8] = PackColor(Src1);
	}
}
