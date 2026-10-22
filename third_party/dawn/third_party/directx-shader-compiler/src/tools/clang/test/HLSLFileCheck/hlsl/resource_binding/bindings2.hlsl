// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: ; Resource Bindings:
// CHECK: ;
// CHECK: ; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
// CHECK: ; ------------------------------ ---------- ------- ----------- ------- -------------- ------
// CHECK: ; buf1                              cbuffer      NA          NA     CB0            cb0unbounded
// CHECK: ; buf3                              cbuffer      NA          NA     CB1     cb8,space3unbounded
// CHECK: ; Samp1                             sampler      NA          NA      S0             s0     1
// CHECK: ; Tex1                              texture     f32          2d      T0             t0     1
// CHECK: ; MyTB                              texture     u32     tbuffer      T1     t11,space3     1
// CHECK: ; tbuf1                             texture     u32     tbuffer      T2            t32     4
// CHECK: ; tbuf3                             texture     u32     tbuffer      T3            t36unbounded
// CHECK: ; tbuf4                             texture     u32     tbuffer      T4             t1    16
// CHECK: ; RWTex1                                UAV     f32          2d      U0             u0     4
// CHECK: ; RWTex3                                UAV     f32          2d      U1             u5unbounded


cbuffer MyCB : register(b11, space3)
{
  float4 f : packoffset(c4);
  float4 f2 : packoffset(c7);
  float fa[3][5] : packoffset(c8);

  tbuffer MyTB : register(t11, space3)
  {
    float f3 : packoffset(c4);
    float4 f4 : packoffset(c7);
    //float4 f_unbounded[] : packoffset(c8);
  };
};

Texture2D<float> Tex1; // t0
Texture2D Tex2[3][7] : register(t11);   // unused
Texture2D<float> Tex3[] : register(t8); // unbounded, unused
Texture2D<float> Tex4[][3] : register(t8, space1); // unbounded, unused

RWTexture2D<float4> RWTex1[4];  // u0
RWTexture2D<float4> RWTex2 : register(u7);
RWTexture2D<float4> RWTex3[] : register(u5);  // unbounded
RWTexture2D<float4> RWTex4[][3] : register(u6, space1);  // unbounded

// Ensure unused explicitly bound does not reserve space:
RWTexture2D<float4> RWUnused[6] : register(u17);

struct Foo
{
  float4 f;
  int4 i;
};

SamplerComparisonState Samp1; // s0
SamplerState Samp2[2][3] : register(s11); // s11
SamplerComparisonState Samp3[] : register(s4); // unused

ConstantBuffer<Foo> buf1[]; // b0, unbounded
ConstantBuffer<Foo> buf2[4][16] : register(b18, space3);  // unused
ConstantBuffer<Foo> buf3[] : register(b8, space3); // unbounded

TextureBuffer<Foo> tbuf1[2][2] : register(t32); // t32
TextureBuffer<Foo> tbuf2[4] : register(t18);  // unused
TextureBuffer<Foo> tbuf3[]; // t36, unbounded
TextureBuffer<Foo> tbuf4[16]; // t1 (fits before unbounded)

float4 main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return (float4)1.0
    // * f  // will overlap with buf3
    * f3   // c4
    * f4   // c7
    // * f_unbounded[13]   // c8 + 13 = c21
    * buf1[2].f // cb0 + 2 = cb2
    // * buf2[2][5].f // cb18 + (2 * 16) + 5 = cb55
    * buf3[27].f // cb8 + 27 = cb35
    * Tex1.SampleCmp(Samp1, coord.xy, coord.z)  // t0, s0
    // * Tex2[2][5].Sample(Samp2[1][2], coord.xy)  // t11 + (2 + 3) + 5 = t22, s11 + (1 * 3) + 2 = s16
    // * Tex3[23].SampleCmp(Samp3[43], coord.xy, coord.z)  // t8 + 23 = t31, s4 + 43 = s47
    * RWTex1[2].Load(a.xy)    // u0 + 2 = u2
    // * RWTex2.Load(a.xy)       // u7
    * RWTex3[18].Load(a.xy)   // u5 + 18 = u23
    * RWTex4[18][1].Load(a.xy)   // u6 + 3*18 + 1 = u61
    * tbuf1[1][1].i // t32 + (1 * 2) + 1 = t35
    // * tbuf2[1].f // t1 + 1 = t2
    * tbuf3[3].f // t36 + 3 = t39
    * tbuf4[7].f // t1 + 7 = t8
    ;
}
