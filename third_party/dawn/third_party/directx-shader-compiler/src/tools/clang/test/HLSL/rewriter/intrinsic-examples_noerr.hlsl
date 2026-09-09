// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

float4 FetchFromIndexMap( uniform Texture2D Tex, uniform SamplerState SS, const float2 RoundedUV, const float LOD )  
{  
    float4 Sample = Tex.SampleLevel( SS, RoundedUV, LOD );  
    return Sample * 255.0f;  
}  

// The following sample includes tex2D mixing new and old style of sampler types.

//--------------------------------------------------------------------------------------
// ATGFont.hlsl
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct VS_IN
{
   float2    Pos    : POSITION;
   uint2    TexAndChannelSelector : TEXCOORD0;                        // u/v packed and 4 bytes packed
};

struct VS_OUT
{
   float4 Position : POSITION;
   float4 Diffuse : COLOR0_center;
   float2 TexCoord0 : TEXCOORD0;
   float4 ChannelSelector : TEXCOORD1;
};


cbuffer OncePerDrawText : register( b0 )
{
    float2    TexScale : register( c0 );
    float4    Color    : register( c1 );
};


// StructuredBuffer should be 't' (it's read-only), and sampler shouldn't have the 't' binding.
//StructuredBuffer< VS_IN >    quadsData : register( u0 ); // expected-warning {{incorrect bind semantic}}
//sampler FontTexture : register(s0) : register( t0 ); // expected-warning {{incorrect bind semantic}}
sampler FontTexture;

VS_OUT FontVertexShader( VS_IN In )
{
    VS_OUT Out;

    Out.Position.x  = In.Pos.x;
    Out.Position.y  = In.Pos.y;
    //Out.Position.z  = ( 0.0 ); // expected-warning {{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
    //Out.Position.w  = ( 1.0 ); // expected-warning {{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
    Out.Diffuse = Color;            // TODO: move to PS simply

    uint texX = (In.TexAndChannelSelector.x >> 0 ) & 0xffff;
    uint texY = (In.TexAndChannelSelector.x >> 16) & 0xffff;
    Out.TexCoord0 = float2( texX, texY ) * TexScale;

    Out.ChannelSelector.w = (0 != (In.TexAndChannelSelector.y & 0xf000)) ? 1 : 0;
    Out.ChannelSelector.x = (0 != (In.TexAndChannelSelector.y & 0x0f00)) ? 1 : 0;
    Out.ChannelSelector.y = (0 != (In.TexAndChannelSelector.y & 0x00f0)) ? 1 : 0;
    Out.ChannelSelector.z = (0 != (In.TexAndChannelSelector.y & 0x000f)) ? 1 : 0;

    return Out;
}

float4 FontPixelShader( VS_OUT In ) : COLOR0
{
    // Fetch a texel from the font texture
    float4 FontTexel = tex2D( FontTexture, In.TexCoord0 ).zyxw;
      
    float4 Color = FontTexel * In.Diffuse;

    if( dot( In.ChannelSelector, 1 ) )
    {
        // Select the color from the channel
        float value = dot( FontTexel, In.ChannelSelector );
          
        // For white pixels, the high bit is 1 and the low
        // bits are luminance, so r0.a will be > 0.5. For the
        // RGB channel, we want to lop off the msb and shift
        // the lower bits up one bit. This is simple to do
        // with the _bx2 modifier. Since these pixels are
        // opaque, we emit a 1 for the alpha channel (which
        // is 0.5 x2 ).
          
        // For black pixels, the high bit is 0 and the low
        // bits are alpha, so r0.a will be < 0.5. For the RGB
        // channel, we emit zero. For the alpha channel, we
        // just use the x2 modifier to scale up the low bits
        // of the alpha.
        Color.rgb = ( value > 0.5f ? 2*value-1 : 0.0f );
        Color.a = 2 * ( value > 0.5f ? 1.0f : value );

        // Return the texture color modulated with the vertex
        // color
        Color *= In.Diffuse;
    }

    clip( Color.a - (8.f / 255.f) );

    return Color;
};
