// RUN: %dxc -Tlib_6_3   -verify -HV 2018 -enable-16bit-types %s

// :FXC_VERIFY_ARGUMENTS: /E FontPixelShader /T ps_5_1 /Gec

float4 FetchFromIndexMap( uniform Texture2D Tex, uniform SamplerState SS, const float2 RoundedUV, const float LOD )
{
    float4 Sample = Tex.SampleLevel( SS, RoundedUV, LOD );
    return Sample * 255.0f;
}

struct S { float f; };

RWByteAddressBuffer uav1 : register(u3);
[shader("pixel")]
float4 RWByteAddressBufferMain(uint2 a : A, uint2 b : B) : SV_Target
{
  float4 r = 0;
  uint status;
  // TODO - fix the following error - the subscript exist, but the indexer type is incorrect - message is misleading
  r += uav1[b]; // expected-error {{type 'RWByteAddressBuffer' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}}
  r += uav1.Load(a); // expected-error {{no matching member function for call to 'Load'}} expected-note {{candidate function template not viable: requires 2 arguments, but 1 was provided}} fxc-error {{X3013:     RWByteAddressBuffer<uint>.Load(uint)}} fxc-error {{X3013:     RWByteAddressBuffer<uint>.Load(uint, out uint status)}} fxc-error {{X3013: 'Load': no matching 1 parameter intrinsic method}} fxc-error {{X3013: Possible intrinsic methods are:}}
  uav1.Load(a, status); // expected-error {{no matching member function for call to 'Load'}} expected-note {{candidate function template not viable: requires single argument 'byteOffset', but 2 arguments were provided}} fxc-error {{X3013:     RWByteAddressBuffer<uint>.Load(uint)}} fxc-error {{X3013:     RWByteAddressBuffer<uint>.Load(uint, out uint status)}} fxc-error {{X3013: 'Load': no matching 2 parameter intrinsic method}} fxc-error {{X3013: Possible intrinsic methods are:}}
  r += status;
  uav1.Load(a, status); // expected-error {{no matching member function for call to 'Load'}} expected-note {{requires single argument 'byteOffset', but 2 arguments were provided}} fxc-error {{X3013:     RWByteAddressBuffer<uint>.Load(uint)}} fxc-error {{X3013:     RWByteAddressBuffer<uint>.Load(uint, out uint status)}} fxc-error {{X3013: 'Load': no matching 2 parameter intrinsic method}} fxc-error {{X3013: Possible intrinsic methods are:}}
  r += status;
  uav1[b] = r; // expected-error {{type 'RWByteAddressBuffer' does not provide a subscript operator}} fxc-error {{X3121: array, matrix, vector, or indexable object type expected in index expression}}
  uav1.Load(a.x, status);
  min16float4 h = min16float4(1,2,3,4);                     /* expected-warning {{'min16float' is promoted to 'half'}} */

  // valid template argument
  r += uav1.Load<half4>(0);
  r += uav1.Load<float4>(12);
  r += uav1.Load<int16_t2>(16).xyxy;
  r += uav1.Load<int32_t3>(20).xyzx;
  r += uav1.Load<float16_t>(20);
  r += uav1.Load<float32_t1>(20);
  r += (float4)uav1.Load<float2x2>(20);
  r += (float4)uav1.Load<float[4]>(20);
  r += uav1.Load<S>(20).f.xxxx;

  r += uav1.Load<half4>(4, status);
  r += uav1.Load<float4>(12, status);
  r += uav1.Load<int16_t2>(16, status).xyxy;
  r += uav1.Load<int32_t3>(20, status).xyzx;
  r += uav1.Load<float16_t>(20, status);
  r += uav1.Load<float32_t1>(20, status);
  r += (float4)uav1.Load<float2x2>(20, status);
  r += (float4)uav1.Load<float[4]>(20, status);
  r += uav1.Load<S>(20, status).f.xxxx;

  // errors
  r += uav1.Load<float, float3>(16);                        /* expected-error {{Explicit template arguments on intrinsic Load must be a single numeric type}} */
  r += uav1.Load<double3>(16);                              /* expected-error {{cannot convert from 'double3' to 'float4'}} */
  r += uav1.Load2<float>(16);                               /* expected-error {{Explicit template arguments on intrinsic Load2 are not supported}} */
  r += uav1.Load3<int>(20);                                 /* expected-error {{Explicit template arguments on intrinsic Load3 are not supported}} */
  r += uav1.Load4<int16_t>(24);                             /* expected-error {{Explicit template arguments on intrinsic Load4 are not supported}} */
  r += uav1.Load<half3x4>(24);                              /* expected-error {{cannot convert from 'half3x4' to 'float4'}} */
  r += uav1.Load<float, float3>(16, status);                /* expected-error {{Explicit template arguments on intrinsic Load must be a single numeric type}} */
  r += uav1.Load<double3>(16, status);                      /* expected-error {{cannot convert from 'double3' to 'float4'}} */
  r += uav1.Load2<float>(16, status);                       /* expected-error {{Explicit template arguments on intrinsic Load2 are not supported}} */
  r += uav1.Load3<int>(20, status);                         /* expected-error {{Explicit template arguments on intrinsic Load3 are not supported}} */
  r += uav1.Load4<int16_t>(24, status);                     /* expected-error {{Explicit template arguments on intrinsic Load4 are not supported}} */
  r += uav1.Load<half3x4>(24, status);                      /* expected-error {{cannot convert from 'half3x4' to 'float4'}} */

  // valid template argument
  struct MyStruct {
    float4 x;
  };
  uav1.Store(0, r);
  uav1.Store(0, r.x);
  uav1.Store(0, (half2)r.xy);
  uav1.Store(0, (int3)r.xyz);
  uav1.Store(0, (double2)r.xy);
  uav1.Store<float>(0, r.x);
  uav1.Store<int64_t4>(0, r);
  uav1.Store(0, float2x4(1,2,3,4,5,6,7,8));
  uav1.Store<float3x2>(0, float3x2(1,2,3,4,5,6));
  uav1.Store(0, (double3)r.xyz);
  uav1.Store(0, (uint64_t4)r);
  uav1.Store(0, (MyStruct)0);
  // errors
  uav1.Store2<float>(0, r.xy);                              /* expected-error {{Explicit template arguments on intrinsic Store2 are not supported}} */
  uav1.Store3<float>(0, r.xyz);                             /* expected-error {{Explicit template arguments on intrinsic Store3 are not supported}} */
  uav1.Store4<float>(0, r);                                 /* expected-error {{Explicit template arguments on intrinsic Store4 are not supported}} */
  return r;
}

// BitCast intrinsics
float4 g_f;
int4 g_i;
uint4 g_u;
float16_t4 g_f16;
int16_t4 g_i16;
uint16_t4 g_u16;
float64_t4 g_f64;
int64_t4 g_i64;
uint64_t4 g_u64;

// TODO: currently compiler is not handling intrinsics correctly after we found overloaded intrinsic function before.
// Uncomment errors below after fixing it.
[shader("pixel")]
float4 BitCastMain() : SV_Target {
  float4 f1 = 0;
  f1 += asfloat(g_f);
  f1 += asfloat(g_i);
  f1 += asfloat(g_u);
  /*
  f1 += asfloat(g_f16);
  f1 += asfloat(g_i16);
  f1 += asfloat(g_u16);
  f1 += asfloat(g_f64);
  f1 += asfloat(g_i64);
  f1 += asfloat(g_u64);
*/
  int4 i1 = 0;
  i1 += asint(g_f);
  i1 += asint(g_i);
  i1 += asint(g_u);
  /*
  i1 += asint(g_f16);
  i1 += asint(g_i16);
  i1 += asint(g_u16);
  i1 += asint(g_f64);
  i1 += asint(g_i64);
  i1 += asint(g_u64);
*/
  uint4 ui1 = 0;
  ui1 += asuint(g_f);
  ui1 += asuint(g_i);
  ui1 += asuint(g_u);
  /*
  ui1 += asuint(g_f16);
  ui1 += asuint(g_i16);
  ui1 += asuint(g_u16);
  ui1 += asuint(g_f64);
  ui1 += asuint(g_i64);
  ui1 += asuint(g_u64);
*/
  float16_t4 f16_1 = 0;
  f16_1 += asfloat16(g_f16);
  f16_1 += asfloat16(g_i16);
  f16_1 += asfloat16(g_u16);
 /*f16_1 += asfloat16(g_f);
  f16_1 += asfloat16(g_i);
  f16_1 += asfloat16(g_u);
  f16_1 += asfloat16(g_f64);
  f16_1 += asfloat16(g_i64);
  f16_1 += asfloat16(g_u64);
  */

  int16_t4 i16_1 = 0;
  i16_1 += asint16(g_f16);
  i16_1 += asint16(g_i16);
  i16_1 += asint16(g_u16);
  /*
  i16_1 += asint16(g_f);
  i16_1 += asint16(g_i);
  i16_1 += asint16(g_u);
  i16_1 += asint16(g_f64);
  i16_1 += asint16(g_i64);
  i16_1 += asint16(g_u64);
*/
  uint16_t4 u16_1 = 0;
  u16_1 += asuint16(g_f16);
  u16_1 += asuint16(g_i16);
  u16_1 += asuint16(g_u16);
  /*
  u16_1 += asuint16(g_f);
  u16_1 += asuint16(g_i);
  u16_1 += asuint16(g_u);
  u16_1 += asuint16(g_f64);
  u16_1 += asuint16(g_i64);
  i16_1 += asuint16(g_u64);
*/
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

// StructuredBuffer should be 't' (it's read-only), and sampler can have the 't' binding.
StructuredBuffer< VS_IN >    quadsData : register( u0 ); // expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}}
sampler FontTexture : register(s0) : register( t0 );

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

float4 FontPixelShader( VS_OUT In ) : COLOR0
{
    // Fetch a texel from the font texture
    float4 FontTexel = tex2D( FontTexture, In.TexCoord0 ).zyxw;
    /*verify-ast
      DeclStmt <col:5, col:63>
      `-VarDecl <col:5, col:59> col:12 used FontTexel 'float4':'vector<float, 4>' cinit
        `-HLSLVectorElementExpr <col:24, col:59> 'vector<float, 4>':'vector<float, 4>' zyxw
          `-CallExpr <col:24, col:57> 'vector<float, 4>':'vector<float, 4>'
            |-ImplicitCastExpr <col:24> 'vector<float, 4> (*)(SamplerState, vector<float, 2>)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:24> 'vector<float, 4> (SamplerState, vector<float, 2>)' lvalue Function 'tex2D' 'vector<float, 4> (SamplerState, vector<float, 2>)'
            |-ImplicitCastExpr <col:31> 'sampler':'SamplerState' <LValueToRValue>
            | `-DeclRefExpr <col:31> 'sampler':'SamplerState' lvalue Var 'FontTexture' 'sampler':'SamplerState'
            `-ImplicitCastExpr <col:44, col:47> 'float2':'vector<float, 2>' <LValueToRValue>
              `-MemberExpr <col:44, col:47> 'float2':'vector<float, 2>' lvalue .TexCoord0
                `-DeclRefExpr <col:44> 'VS_OUT' lvalue ParmVar 'In' 'VS_OUT'
    */

    float4 Color = FontTexel * In.Diffuse;
    float dot_result = dot( In.ChannelSelector, 1 );
    /*verify-ast
      DeclStmt <col:5, col:52>
      `-VarDecl <col:5, col:51> col:11 dot_result 'float' cinit
        `-CallExpr <col:24, col:51> 'float'
          |-ImplicitCastExpr <col:24> 'float (*)(vector<float, 4>, vector<float, 4>)' <FunctionToPointerDecay>
          | `-DeclRefExpr <col:24> 'float (vector<float, 4>, vector<float, 4>)' lvalue Function 'dot' 'float (vector<float, 4>, vector<float, 4>)'
          |-ImplicitCastExpr <col:29, col:32> 'float4':'vector<float, 4>' <LValueToRValue>
          | `-MemberExpr <col:29, col:32> 'float4':'vector<float, 4>' lvalue .ChannelSelector
          |   `-DeclRefExpr <col:29> 'VS_OUT' lvalue ParmVar 'In' 'VS_OUT'
          `-ImplicitCastExpr <col:49> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
            `-ImplicitCastExpr <col:49> 'float' <IntegralToFloating>
              `-IntegerLiteral <col:49> 'literal int' 1
    */
    int litint_abs = abs( 20 );
    /*verify-ast
      DeclStmt <col:5, col:31>
      `-VarDecl <col:5, col:30> col:9 litint_abs 'int' cinit
        `-ImplicitCastExpr <col:22, col:30> 'int' <IntegralCast>
          `-CallExpr <col:22, col:30> 'literal int'
            |-ImplicitCastExpr <col:22> 'literal int (*)(literal int)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:22> 'literal int (literal int)' lvalue Function 'abs' 'literal int (literal int)'
            `-IntegerLiteral <col:27> 'literal int' 20
    */
    int litint_abs2 = abs( -50 );
    /*verify-ast
      DeclStmt <col:5, col:33>
      `-VarDecl <col:5, col:32> col:9 litint_abs2 'int' cinit
        `-ImplicitCastExpr <col:23, col:32> 'int' <IntegralCast>
          `-CallExpr <col:23, col:32> 'literal int'
            |-ImplicitCastExpr <col:23> 'literal int (*)(literal int)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:23> 'literal int (literal int)' lvalue Function 'abs' 'literal int (literal int)'
            `-UnaryOperator <col:28, col:29> 'literal int' prefix '-'
              `-IntegerLiteral <col:29> 'literal int' 50
    */
    float litfloat_abs = abs( -1.5 );
    /*verify-ast
      DeclStmt <col:5, col:37>
      `-VarDecl <col:5, col:36> col:11 litfloat_abs 'float' cinit
        `-ImplicitCastExpr <col:26, col:36> 'float' <FloatingCast>
          `-CallExpr <col:26, col:36> 'literal float'
            |-ImplicitCastExpr <col:26> 'literal float (*)(literal float)' <FunctionToPointerDecay>
            | `-DeclRefExpr <col:26> 'literal float (literal float)' lvalue Function 'abs' 'literal float (literal float)'
            `-UnaryOperator <col:31, col:32> 'literal float' prefix '-'
              `-FloatingLiteral <col:32> 'literal float' 1.500000e+00
    */
    // Note: the following is ambiguous because literal-ness is preserved through the abs()
    // intrinsic, and the multiply binop, resulting in a 'literal float' that could map either
    // to float or double.
    float ambiguous = overload1(1.0 * abs(-1));      /* expected-error {{call to 'overload1' is ambiguous}} fxc-error {{X3067: 'overload1': ambiguous function call}} */

    if( dot( In.ChannelSelector, 1 ) )
    {
        // Select the color from the channel
        float value = dot( FontTexel, In.ChannelSelector );
        /*verify-ast
          DeclStmt <col:9, col:59>
          `-VarDecl <col:9, col:58> col:15 used value 'float' cinit
            `-CallExpr <col:23, col:58> 'float'
              |-ImplicitCastExpr <col:23> 'float (*)(vector<float, 4>, vector<float, 4>)' <FunctionToPointerDecay>
              | `-DeclRefExpr <col:23> 'float (vector<float, 4>, vector<float, 4>)' lvalue Function 'dot' 'float (vector<float, 4>, vector<float, 4>)'
              |-ImplicitCastExpr <col:28> 'float4':'vector<float, 4>' <LValueToRValue>
              | `-DeclRefExpr <col:28> 'float4':'vector<float, 4>' lvalue Var 'FontTexel' 'float4':'vector<float, 4>'
              `-ImplicitCastExpr <col:39, col:42> 'float4':'vector<float, 4>' <LValueToRValue>
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
      |-ImplicitCastExpr <col:5> 'void (*)(float)' <FunctionToPointerDecay>
      | `-DeclRefExpr <col:5> 'void (float)' lvalue Function 'clip' 'void (float)'
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
