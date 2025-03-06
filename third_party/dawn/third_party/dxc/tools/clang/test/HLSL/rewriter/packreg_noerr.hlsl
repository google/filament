// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s

// To test with the classic compiler, run
// %sdxroot%\tools\x86\fxc.exe /T ps_5_1 packreg.hlsl

// fxc error X3115: Conflicting register semantics: 's0' and 's1'
//sampler myVar_conflict : register(s0) : register(s1); // expected-error {{conflicting register semantics}} fxc-error {{X3115: Conflicting register semantics: 's0' and 's1'}} 
//float4 f_conflict : register(c0) : register(c1); // expected-error {{conflicting register semantics}} fxc-error {{X3115: Conflicting register semantics: 'c0' and 'c1'}} 
float4 f_no_conflict : register(vs, c0) : register(ps, c1);

// fxc error X3530: invalid register specification, expected 'b' or 'c' binding
//cbuffer MySamplerBuffer : register(s20) // expected-error {{invalid register specification, expected 'b' or 'c' binding}} fxc-error {{X3530: invalid register specification, expected 'b' or 'c' binding}} 
//{
//  float4 MySamplerBuffer_f4;
//}

// fxc error X4567: maximum cbuffer exceeded. target has 14 slots, manual bind to slot 20 failed
// We leave this bit of validation to the back-end, as this is presumably target-dependent.
//cbuffer MyLargeBBuffer : register(b20)
//{
//  float4 MyLargeBBuffer_f4;
//}

// fxc error X3515: User defined constant buffer slots cannot be target specific
//cbuffer MyUDBuffer : register(ps, b0) : register(vs, b2)  // expected-error {{user defined constant buffer slots cannot be target specific}} expected-error {{user defined constant buffer slots cannot be target specific}} fxc-error {{X3515: User defined constant buffer slots cannot be target specific}} fxc-error {{X3530: Buffers may only be bound to one slot.}} 
//{
//  float4 MyUDBuffer_f4;
//}

// fxc error X3530: Buffers may only be bound to one slot.
//cbuffer MyDupeBuffer2 : register(b0) : register(b2) // expected-error {{conflicting register semantics}} fxc-error {{X3530: Buffers may only be bound to one slot.}} 
//{
//  float4 Element100 : packoffset(c100) : MY_SEMANTIC : packoffset(c0) : packoffset(c2); // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}} 
//}

// fxc error X3530: Buffers may only be bound to one constant offset.
//cbuffer MyDupeBuffer : register(c0) : register(c1) // expected-error {{conflicting register semantics}} fxc-error {{X3530: Buffers may only be bound to one constant offset.}} 
//{
//  float4 Element101 : packoffset(c100) : MY_SEMANTIC : packoffset(c0) : packoffset(c2); // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}} 
//}

cbuffer MyFloats
{
  float4 f4_simple : packoffset(c0.x);
  // fxc error X3530: register or offset bind c3.xy not valid
  //float4 f4_nonseq : packoffset(c2.xy); // expected-error {{packoffset component should indicate offset with one of x, y, z, w, r, g, b, or a}} fxc-error {{X3530: register or offset bind c2.xy not valid}} 
  //float4 f4_outoforder : packoffset(c3.w); // expected-error {{register or offset bind not valid}} fxc-error {{X3530: register or offset bind c3.w not valid}} 
}

tbuffer OtherFloats
{
  float4 f4_t_simple : packoffset(c10.x);
  float3 f3_t_simple : packoffset(c11.y);
  float2 f2_t_simple : packoffset(c12.z);
  // fxc error X3530: register or offset bind c3.xy not valid
  //float4 f4_t_nonseq : packoffset(c12.xy); // expected-error {{packoffset component should indicate offset with one of x, y, z, w, r, g, b, or a}} fxc-error {{X3530: register or offset bind c12.xy not valid}} 
  // fxc error X3530: register or offset bind c3.w not valid
  //float4 f4_t_outoforder : packoffset(c13.w); // expected-error {{register or offset bind not valid}} fxc-error {{X3530: register or offset bind c13.w not valid}} 
}

//sampler myvar_noparens : register; // expected-error {{expected '(' after 'register'}} fxc-error {{X3000: syntax error: unexpected token ';'}} 
//sampler myvar_noclosebracket: register(ps, s[2); // expected-error {{expected ']'}} expected-note {{to match this '['}} fxc-error {{X3000: syntax error: unexpected token ')'}} 
//sampler myvar_norparen: register(ps, s[2]; // expected-error {{expected ')'}} fxc-pass {{}} 
sampler myVar : register(ps_6_0, s);
sampler myVar2 : register(vs, s[8]);
sampler myVar2_offset : register(vs, s2[8]);
//sampler myVar2_emptyu : register(vs, s2[]); // expected-error {{expected expression}} fxc-pass {{}} 
sampler myVar_2 : register(vs, s8);
// fxc error: error X4017: cannot bind the same variable to multiple constants in the same constant bank
//sampler myVar3 : register(ps_6_0, s[0]) : register(vs, s[8]);
// fxc error X3591: incorrect bind semantic
//sampler myVar4 : register(vs, t0); // expected-warning {{incorrect bind semantic}} fxc-pass {{}} 
sampler myVar65536 : register(vs, s65536);
//sampler myVar4294967296 : register(vs, s4294967296); // expected-error {{register number should be an integral numeric string}} fxc-pass {{}} 
//sampler myVar281474976710656 : register(vs, s281474976710656); // expected-error {{register number should be an integral numeric string}} fxc-pass {{}} 
sampler myVar5 : register(vs, s);
// fxc error X3530: register vs not valid
//sampler myVar6 : register(vs); // expected-error {{expected ','}} fxc-pass {{}} 
// fxc error X3000: syntax error: unexpected token ')'
//sampler myVar7 : register(); // expected-error {{expected identifier}} fxc-pass {{}} 
// fxc error X3000: syntax error: unexpected token ';'
//sampler myVar8 : ; // expected-error {{expected ';' after top level declarator}} fxc-pass {{}} 
// fxc error X3000: error X3530: register a0 not valid
//sampler myVar9 : register(ps, a0); // expected-error {{register type is unsupported - valid values are 'b', 'c', 'i', 's', 't'}} fxc-pass {{}} 
// fxc error X3000: error X3530: register a not valid
//sampler myVar10 : register(ps, a); // expected-error {{register type is unsupported - valid values are 'b', 'c', 'i', 's', 't'}} fxc-pass {{}} 
AppendStructuredBuffer<float4> myVar11 : register(ps, u1);
//Buffer<float4> myVar11_plain : register(ps, u2); 
RWStructuredBuffer<float4> myVar11_rw : register(ps, u);
// fxc error X3000: syntax error: unexpected integer constant
//sampler myVar12 : register(ps, 3); // expected-error {{expected identifier}} fxc-pass {{}} 
// fxc error X3000: error X3530: register _ not valid
//sampler myVar13 : register(ps, _); // expected-error {{register type is unsupported - valid values are 'b', 'c', 'i', 's', 't'}} fxc-pass {{}} 
// fxc error X3091: packoffset is only allowed in a constant buffer
//sampler myVar14 : packoffset(c0); // expected-error {{packoffset is only allowed in a constant buffer}} fxc-pass {{}} 
sampler myVar_s : register(ps, s);
Texture2D myVar_t : register(ps, t);
Texture2D myVar_t_1 : register(ps, t[1]);
Texture2D myVar_t_1_1 : register(ps, t1[1]);
// fxc error X3591: incorrect bind semantic
//sampler myVar_i : register(ps, i); // expected-warning {{incorrect bind semantic}} fxc-pass {{}} 
float4 myVar_b : register(ps, b);
bool myVar_bool : register(ps, b) : register(ps, c);
// fxc error X3591: incorrect bind semantic
//sampler myVar_c : register(ps, c); // expected-warning {{incorrect bind semantic}} fxc-pass {{}} 
// fxc error X3000: error X3530: register z not valid
//sampler myVar_z : register(ps, z); // expected-error {{register type is unsupported - valid values are 'b', 'c', 'i', 's', 't'}} fxc-pass {{}} 
sampler myVar_1 : register(ps, s[1]);
sampler myVar_11 : register(ps, s[1+1]);
sampler myVar_16 : register(ps, s[15]);
// fxc error X4509: maximum sampler register index exceeded, target has 16 slots, manual bind to slot s1073741823 failed
//sampler myVar_n1 : register(ps, s[-1]); // expected-error {{register subcomponent is not an integral constant}} fxc-pass {{}} 
sampler myVar_n1p5 : register(ps, s[1.5]);
sampler myVar_s1 : register(ps, s[1], space1);
// fxc error X3591: incorrect bind semantic
//sampler myVar_sz : register(ps, s[1], spacez); // expected-error {{space number should be an integral numeric string}} fxc-pass {{}} 

cbuffer MyBuffer
{
  float4 Element1 : packoffset(c0);
  float1 Element2 : packoffset(c1);
  float1 Element3 : packoffset(c1.y);
  float4 Element4 : packoffset(c10) : packoffset(c10);
  //float4 Element5 : packoffset(c10) : MY_SEMANTIC : packoffset(c11); // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}} 
}

Texture2D<float4> Texture_ : register(t0);
sampler Sampler : register(s0);

cbuffer Parameters : register(b0)
{
  float4   DiffuseColor   : packoffset(c0) : register(c0);
  float4   AlphaTest      : packoffset(c1);
  float3   FogColor       : packoffset(c2);
  float4   FogVector      : packoffset(c3);
  float4x4 WorldViewProj  : packoffset(c4);
  //float4x4 WorldViewProj2  : packoffset; // expected-error {{expected '(' after 'packoffset'}} fxc-pass {{}} 
  //float4x4 WorldViewProj3  : packoffset(1); // expected-error {{expected identifier}} fxc-pass {{}} 
  //float4x4 WorldViewProj4  : packoffset(c4.1);  // expected-error {{expected ')'}} fxc-pass {{}} 
};

cbuffer cbPerObject : register(b0)
{
  float4		g_vObjectColor			: packoffset(c0);
};

cbuffer cbPerFrame : register(b1)
{
  float3		g_vLightDir				: packoffset(c0);
  float		g_fAmbient : packoffset(c0.w);
};

// Nesting case
cbuffer OuterBuffer {
  float OuterItem0;
  cbuffer InnerBuffer {
    float InnerItem0;
  };
  float OuterItem1;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register(t0);
SamplerState g_samLinear : register(s0);

float2 f2() {
  g_txDiffuse.Sample(myVar5, float2(1, 2));
  return 0;
}

float4 main(float4 param4 : TEXCOORD0) : SV_Target0 {
// int main() {
  float f = OuterItem0 + OuterItem1 + InnerItem0;
  return g_txDiffuse.Sample(myVar_s, float2(1, f));
}