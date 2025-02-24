// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// :FXC_VERIFY_ARGUMENTS: /E main /T ps_5_1

// CHECK: @main
float4 f_no_conflict : register(vs, c0) : register(ps, c1);

cbuffer MySamplerBuffer : register(b25)
{
  float4 MySamplerBuffer_f4;
}

cbuffer CB3 : register(b3) { float c3; }

tbuffer TB11 : register(t11) { float c11; }

// We leave this bit of validation to the back-end, as this is presumably target-dependent.
cbuffer MyLargeBBuffer : register(b20)
{
  float4 MyLargeBBuffer_f4;
}

cbuffer MySamplerBuffer : register(b21)
{
  float4 MyUDBuffer_f4;
}

cbuffer MyDupeBuffer2 : register(b22)
{
  float4 Element100 : packoffset(c100) : MY_SEMANTIC : packoffset(c0) : packoffset(c2); // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}}
}

cbuffer MyDupeBuffer : register(b23)
{
  float4 Element101 : packoffset(c100) : MY_SEMANTIC : packoffset(c0) : packoffset(c2); // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}}
}

cbuffer MyFloats
{
  float4 f4_simple : packoffset(c0.x);
  /*verify-ast
    VarDecl <col:3, col:10> f4_simple 'float4':'vector<float, 4>'
    |-ConstantPacking <col:22> packoffset(c0.x)
  */
}

tbuffer OtherFloats
{
  float4 f4_t_simple : packoffset(c10.x);
  float3 f3_t_simple : packoffset(c11.y);
  /*verify-ast
    VarDecl <col:3, col:10> f3_t_simple 'float3':'vector<float, 3>'
    |-ConstantPacking <col:24> packoffset(c11.y)
  */
}

sampler myVar : register(ps_6_0, s);
/*verify-ast
  VarDecl <col:1, col:9> myVar 'sampler':'SamplerState'
  |-RegisterAssignment <col:17> register(ps_6_0, s0)
*/
sampler myVar2 : register(vs, s[8]);
sampler myVar2_offset : register(vs, s2[8]);
/*verify-ast
  VarDecl <col:1, col:9> myVar2_offset 'sampler':'SamplerState'
  |-RegisterAssignment <col:25> register(vs, s10)
*/
sampler myVar_2 : register(vs, s8);
sampler myVar3 : register(ps_6_0, s[0]) : register(vs, s[8]);
sampler myVar4 : register(vs, t0); // expected-warning {{incorrect bind semantic}} fxc-pass {{}}
sampler myVar65536 : register(vs, s65536);
sampler myVar281474976710656 : register(vs, s10656); 

sampler myVar5 : register(vs, s);
/*verify-ast
  VarDecl <col:1, col:9> myVar5 'sampler':'SamplerState'
  |-RegisterAssignment <col:18> register(vs, s0)
*/
AppendStructuredBuffer<float4> myVar11 : register(ps, u1);
RWStructuredBuffer<float4> myVar11_rw : register(ps, u);
sampler myVar_s : register(ps, s);
Texture2D myVar_t : register(ps, t);
Texture2D myVar_t_1 : register(ps, t[1]);
Texture2D myVar_t_1_1 : register(ps, t1[1]), 
/*verify-ast
  VarDecl <col:1, col:11> myVar_t_1_1 'Texture2D':'Texture2D<vector<float, 4> >'
  |-RegisterAssignment <col:25> register(ps, t2)
  VarDecl <col:1, line:133:3> myVar_t_2_1 'Texture2D':'Texture2D<vector<float, 4> >'
  |-RegisterAssignment <col:17> register(ps, t3)
  |-RegisterAssignment <col:39> register(vs, t0)
*/
  myVar_t_2_1 : register(ps, t2[1]) : register(vs, t0);

float4 myVar_b : register(ps, b);
bool myVar_bool : register(ps, b) : register(ps, c);
/*verify-ast
  VarDecl <col:1, col:6> myVar_bool 'bool'
  |-RegisterAssignment <col:19> register(ps, b0)
  |-RegisterAssignment <col:37> register(ps, c0)
*/
sampler myVar_1 : register(ps, s[1]);
sampler myVar_11 : register(ps, s[1+1]);
/*verify-ast
  VarDecl <col:1, col:9> myVar_11 'sampler':'SamplerState'
  |-RegisterAssignment <col:20> register(ps, s2)
*/
sampler myVar_16 : register(ps, s[15]);
sampler myVar_n1p5 : register(ps, s[1.5]);
sampler myVar_s1 : register(ps, s[1], space1);
/*verify-ast
  VarDecl <col:1, col:9> myVar_s1 'sampler':'SamplerState'
  |-RegisterAssignment <col:20> register(ps, s1, space1)
*/

cbuffer MyBuffer
{
  float4 Element1 : packoffset(c0);
  float1 Element2 : packoffset(c1);
  float1 Element3 : packoffset(c1.y);
  float4 Element4 : packoffset(c10) : packoffset(c10);
  float4 Element5 : packoffset(c10) : MY_SEMANTIC : packoffset(c11), Element6 : packoffset(c12) : MY_SEMANTIC2; // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}}
  /*verify-ast
    VarDecl <col:3, col:10> Element5 'float4':'vector<float, 4>'
    |-ConstantPacking <col:21> packoffset(c10.x)
    |-SemanticDecl <col:39> "MY_SEMANTIC"
    |-ConstantPacking <col:53> packoffset(c11.x)
    VarDecl <col:3, col:70> Element6 'float4':'vector<float, 4>'
    |-ConstantPacking <col:81> packoffset(c12.x)
    |-SemanticDecl <col:99> "MY_SEMANTIC2"
  */
}

// Cannot name this variable 'Texture' in dxc, as it conflicts with effect deprecated
// object types.  Fxc is ok with it, however.
// expected-note@? {{previous definition is here}}
Texture2D<float4> Texture_ : register(t0);
sampler Sampler : register(s0);

cbuffer Parameters : register(b0)
{
  float4   DiffuseColor   : packoffset(c0) : register(c0);
  /*verify-ast
    VarDecl <col:3, col:12> DiffuseColor 'float4':'vector<float, 4>'
    |-ConstantPacking <col:29> packoffset(c0.x)
    |-RegisterAssignment <col:46> register(c0)
  */
  float4   AlphaTest      : packoffset(c1);
  float3   FogColor       : packoffset(c2);
  float4   FogVector      : packoffset(c3);
  float4x4 WorldViewProj  : packoffset(c4);
};

cbuffer cbPerObject : register(b0)
{
  float4    g_vObjectColor      : packoffset(c0);
};

cbuffer cbPerFrame : register(b1)
{
  float3    g_vLightDir       : packoffset(c0);
  float   g_fAmbient : packoffset(c0.w);
};

// Nesting case
cbuffer OuterBuffer : register(b3) {
/*verify-ast
  HLSLBufferDecl <col:1, line:236:1> cbuffer OuterBuffer
  |-RegisterAssignment <col:23> register(b3)
  |-VarDecl <line:231:3, col:9> OuterItem0 'float'
  |-HLSLBufferDecl parent <line:232:3, line:234:3> cbuffer InnerBuffer
  | |-RegisterAssignment <col:25> register(b4)
  | `-VarDecl <line:233:5, col:11> InnerItem0 'float'
  |-EmptyDecl <line:234:4>
  `-VarDecl <line:235:3, col:9> OuterItem1 'float'
*/
  float OuterItem0;
  cbuffer InnerBuffer : register(b4) {
    float InnerItem0;
  };
  float OuterItem1;
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D g_txDiffuse : register(t0);
SamplerState g_samLinear : register(s0);

float2 f2() {
  g_txDiffuse.Sample(myVar281474976710656, float2(1, 2));
  return 0;
}

float4 main(float4 param4 : TEXCOORD0) : SV_Target0 {
// int main() {
  float f = OuterItem0 + OuterItem1 + InnerItem0;
  return g_txDiffuse.Sample(myVar_s, float2(1, f));
}