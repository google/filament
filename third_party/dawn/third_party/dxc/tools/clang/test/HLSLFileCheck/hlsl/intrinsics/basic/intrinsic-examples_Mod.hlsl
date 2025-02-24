// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: dot4
// CHECK: discard

// :FXC_VERIFY_ARGUMENTS: /E FontPixelShader /T ps_5_1 /Gec

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

float overload1(float f) { return 1.0f; }                   /* expected-note {{candidate function}} fxc-pass {{}} */
double overload1(double d) { return 1.0l; }                 /* expected-note {{candidate function}} fxc-pass {{}} */


VS_OUT FontVertexShader( VS_IN In )
{
    VS_OUT Out;

    Out.Position.x  = In.Pos.x;
    Out.Position.y  = In.Pos.y;
    Out.Position.z  = ( 0.0 );
    Out.Position.w  = ( 1.0 );
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

SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

float4 main( VS_OUT In ) : SV_Target
{
    // Fetch a texel from the font texture
    float4 FontTexel = text1.Sample(samp1, In.TexCoord0).zyxw;
    /*verify-ast
      DeclStmt <col:5, col:63>
      `-VarDecl <col:5, col:59> FontTexel 'float4':'vector<float, 4>'
        `-HLSLVectorElementExpr <col:24, col:59> 'vector<float, 4>':'vector<float, 4>' zyxw
          `-CallExpr <col:24, col:57> 'vector<float, 4>':'vector<float, 4>'
            |-ImplicitCastExpr <col:24> 'vector<float, 4> (*)(const SamplerState &, const vector<float, 2> &)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:24> 'vector<float, 4> (const SamplerState &, const vector<float, 2> &)' lvalue Function 'tex2D' 'vector<float, 4> (const SamplerState &, const vector<float, 2> &)'
            |-DeclRefExpr <col:31> 'sampler':'SamplerState' lvalue Var 'FontTexture' 'sampler':'SamplerState'
            `-MemberExpr <col:44, col:47> 'float2':'vector<float, 2>' lvalue .TexCoord0
              `-DeclRefExpr <col:44> 'VS_OUT' lvalue ParmVar 'In' 'VS_OUT'
    */

    float4 Color = FontTexel * In.Diffuse;
    float dot_result = dot( In.ChannelSelector, 1 );
    /*verify-ast
      DeclStmt <col:5, col:52>
      `-VarDecl <col:5, col:51> dot_result 'float'
        `-CallExpr <col:24, col:51> 'float'
          |-ImplicitCastExpr <col:24> 'float (*)(const vector<float, 4> &, const vector<float, 4> &)' <FunctionToPointerDecay>
          | `-DeclRefExpr <col:24> 'float (const vector<float, 4> &, const vector<float, 4> &)' lvalue Function 'dot' 'float (const vector<float, 4> &, const vector<float, 4> &)'
          |-MemberExpr <col:29, col:32> 'float4':'vector<float, 4>' lvalue .ChannelSelector
          | `-DeclRefExpr <col:29> 'VS_OUT' lvalue ParmVar 'In' 'VS_OUT'
          `-ImplicitCastExpr <col:49> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
            `-ImplicitCastExpr <col:49> 'float' <IntegralToFloating>
              `-IntegerLiteral <col:49> 'literal int' 1
    */
    int litint_abs = abs( 20 );
    /*verify-ast
      DeclStmt <col:5, col:31>
      `-VarDecl <col:5, col:30> litint_abs 'int'
        `-ImplicitCastExpr <col:22, col:30> 'int' <IntegralCast>
          `-CallExpr <col:22, col:30> 'literal int'
            |-ImplicitCastExpr <col:22> 'literal int (*)(const literal int &)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:22> 'literal int (const literal int &)' lvalue Function 'abs' 'literal int (const literal int &)'
            `-IntegerLiteral <col:27> 'literal int' 20
    */
    int litint_abs2 = abs( -50 );
    /*verify-ast
      DeclStmt <col:5, col:33>
      `-VarDecl <col:5, col:32> litint_abs2 'int'
        `-ImplicitCastExpr <col:23, col:32> 'int' <IntegralCast>
          `-CallExpr <col:23, col:32> 'literal int'
            |-ImplicitCastExpr <col:23> 'literal int (*)(const literal int &)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:23> 'literal int (const literal int &)' lvalue Function 'abs' 'literal int (const literal int &)'
            `-UnaryOperator <col:28, col:29> 'literal int' prefix '-'
              `-IntegerLiteral <col:29> 'literal int' 50
    */
    float litfloat_abs = abs( -1.5 );
    /*verify-ast
      DeclStmt <col:5, col:37>
      `-VarDecl <col:5, col:36> litfloat_abs 'float'
        `-ImplicitCastExpr <col:26, col:36> 'float' <FloatingCast>
          `-CallExpr <col:26, col:36> 'literal float'
            |-ImplicitCastExpr <col:26> 'literal float (*)(const literal float &)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:26> 'literal float (const literal float &)' lvalue Function 'abs' 'literal float (const literal float &)'
            `-UnaryOperator <col:31, col:32> 'literal float' prefix '-'
              `-FloatingLiteral <col:32> 'literal float' 1.500000e+00
    */
    // Note: the following is ambiguous because literal-ness is preserved through the abs()
    // intrinsic, and the multiply binop, resulting in a 'literal float' that could map either
    // to float or double.

    if( dot( In.ChannelSelector, 1 ) )
    {
        // Select the color from the channel
        float value = dot( FontTexel, In.ChannelSelector );
        /*verify-ast
          DeclStmt <col:9, col:59>
          `-VarDecl <col:9, col:58> value 'float'
            `-CallExpr <col:23, col:58> 'float'
              |-ImplicitCastExpr <col:23> 'float (*)(const vector<float, 4> &, const vector<float, 4> &)' <FunctionToPointerDecay>
              | `-DeclRefExpr <col:23> 'float (const vector<float, 4> &, const vector<float, 4> &)' lvalue Function 'dot' 'float (const vector<float, 4> &, const vector<float, 4> &)'
              |-DeclRefExpr <col:28> 'float4':'vector<float, 4>' lvalue Var 'FontTexel' 'float4':'vector<float, 4>'
              `-MemberExpr <col:39, col:42> 'float4':'vector<float, 4>' lvalue .ChannelSelector
                `-DeclRefExpr <col:39> 'VS_OUT' lvalue ParmVar 'In' 'VS_OUT'
        */

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
    /*verify-ast
      CallExpr <col:5, col:35> 'void'
      |-ImplicitCastExpr <col:5> 'void (*)(const float &)' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:5> 'void (const float &)' lvalue Function 'clip' 'void (const float &)'
      `-BinaryOperator <col:11, col:33> 'float' '-'
        |-ImplicitCastExpr <col:11, col:17> 'float' <LValueToRValue>
        | `-HLSLVectorElementExpr <col:11, col:17> 'float' lvalue vectorcomponent a
        |   `-DeclRefExpr <col:11> 'float4':'vector<float, 4>' lvalue Var 'Color' 'float4':'vector<float, 4>'
        `-ParenExpr <col:21, col:33> 'float'
          `-BinaryOperator <col:22, col:28> 'float' '/'
            |-FloatingLiteral <col:22> 'float' 8.000000e+00
            `-FloatingLiteral <col:28> 'float' 2.550000e+02
    */

    return Color;
};