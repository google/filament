// RUN: %dxc -Tlib_6_3   -verify %s
// RUN: %dxc -Tps_6_0   -verify %s
// :FXC_VERIFY_ARGUMENTS: /E main /T ps_5_1

// fxc error X3115: Conflicting register semantics: 's0' and 's1'
sampler myVar_conflict : register(s0) : register(s1); // expected-error {{conflicting register semantics}} fxc-error {{X3115: Conflicting register semantics: 's0' and 's1'}}
float4 f_conflict : register(c0) : register(c1); // expected-error {{conflicting register semantics}} fxc-error {{X3115: Conflicting register semantics: 'c0' and 'c1'}}
float4 f_no_conflict : register(vs, c0) : register(ps, c1);

// fxc error X3530: invalid register specification, expected 'b' or 'c' binding
cbuffer MySamplerBuffer : register(s20) // expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}}
{
  float4 MySamplerBuffer_f4;
}

struct Foo {
  float g1;
};

cbuffer CB1 : register(t1) { float c1; }                    /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
cbuffer CB2 : register(c2) { float c2; }                    /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
cbuffer CB3 : register(b3) { float c3; }
cbuffer CB4 : register(u4) { float c4; }                    /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
ConstantBuffer<Foo> D3D12CB1 : register(t1);                /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
ConstantBuffer<Foo> D3D12CB2 : register(c1);                /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
ConstantBuffer<Foo> D3D12CB3 : register(b1);
ConstantBuffer<Foo> D3D12CB4 : register(u1);                /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */

cbuffer CB5 : register(T1) { float c5; }                    /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
cbuffer CB6 : register(C2) { float c6; }                    /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
cbuffer CB7 : register(B3) { float c7; }
cbuffer CB8 : register(U4) { float c8; }                    /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
ConstantBuffer<Foo> D3D12CB5 : register(T1);                /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
ConstantBuffer<Foo> D3D12CB6 : register(C1);                /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */
ConstantBuffer<Foo> D3D12CB7 : register(B1);
ConstantBuffer<Foo> D3D12CB8 : register(U1);                /* expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}} */

tbuffer TB11 : register(t11) { float c11; }
tbuffer TB12 : register(c12) { float c12; }                 /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
tbuffer TB13 : register(b13) { float c13; }                 /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
tbuffer TB14 : register(u14) { float c14; }                 /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
TextureBuffer<Foo> D3D12TB11 : register(t11);
TextureBuffer<Foo> D3D12TB12 : register(c11);               /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
TextureBuffer<Foo> D3D12TB13 : register(b11);               /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
TextureBuffer<Foo> D3D12TB14 : register(u11);               /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */

tbuffer TB15 : register(T11) { float c15; }
tbuffer TB16 : register(C12) { float c16; }                 /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
tbuffer TB17 : register(B13) { float c17; }                 /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
tbuffer TB18 : register(U14) { float c18; }                 /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
TextureBuffer<Foo> D3D12TB15 : register(T11);
TextureBuffer<Foo> D3D12TB16 : register(C11);               /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
TextureBuffer<Foo> D3D12TB17 : register(B11);               /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */
TextureBuffer<Foo> D3D12TB18 : register(U11);               /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3591: incorrect bind semantic}} */

// fxc error X4567: maximum cbuffer exceeded. target has 14 slots, manual bind to slot 20 failed
// We leave this bit of validation to the back-end, as this is presumably target-dependent.
cbuffer MyLargeBBuffer : register(b20)
{
  float4 MyLargeBBuffer_f4;
}

// fxc error X3515: User defined constant buffer slots cannot be target specific
cbuffer MyUDBuffer : register(ps, b0) : register(vs, b2)  // expected-error {{user defined constant buffer slots cannot be target specific}} expected-error {{user defined constant buffer slots cannot be target specific}} fxc-error {{X3515: User defined constant buffer slots cannot be target specific}} fxc-error {{X3530: Buffers may only be bound to one slot.}}
{
  float4 MyUDBuffer_f4;
}

// fxc error X3530: Buffers may only be bound to one slot.
cbuffer MyDupeBuffer2 : register(b0) : register(b2) // expected-error {{conflicting register semantics}} fxc-error {{X3530: Buffers may only be bound to one slot.}}
{
  float4 Element100 : packoffset(c100) : MY_SEMANTIC : packoffset(c0) : packoffset(c2); // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}}
}

// fxc error X3530: Buffers may only be bound to one constant offset.
cbuffer MyDupeBuffer : register(c0) : register(c1) // expected-error {{conflicting register semantics}} expected-error {{invalid register specification, expected 'b' binding}} expected-error {{invalid register specification, expected 'b' binding}} fxc-error {{X3591: incorrect bind semantic}}
{
  float4 Element101 : packoffset(c100) : MY_SEMANTIC : packoffset(c0) : packoffset(c2); // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}}
}

cbuffer MyFloats
{
  float4 f4_simple : packoffset(c0.x);
  /*verify-ast
    VarDecl parent cbuffer <col:3, col:10> col:10 f4_simple 'const float4':'const vector<float, 4>'
    `-ConstantPacking <col:22> packoffset(c0.x)
  */
  // fxc error X3530: register or offset bind c3.xy not valid
  float4 f4_nonseq : packoffset(c2.xy); // expected-error {{packoffset component should indicate offset with one of x, y, z, w, r, g, b, or a}} fxc-error {{X3530: register or offset bind c2.xy not valid}}
  float4 f4_outoforder : packoffset(c3.w); // expected-error {{register or offset bind not valid}} fxc-error {{X3530: register or offset bind c3.w not valid}}
}

tbuffer OtherFloats
{
  float4 f4_t_simple : packoffset(c10.x);
  float3 f3_t_simple : packoffset(c11.y);
  /*verify-ast
    VarDecl parent tbuffer <col:3, col:10> col:10 f3_t_simple 'const float3':'const vector<float, 3>'
    `-ConstantPacking <col:24> packoffset(c11.y)
  */
  // fxc error X3530: register or offset bind c3.xy not valid
  float4 f4_t_nonseq : packoffset(c12.xy); // expected-error {{packoffset component should indicate offset with one of x, y, z, w, r, g, b, or a}} fxc-error {{X3530: register or offset bind c12.xy not valid}}
  // fxc error X3530: register or offset bind c3.w not valid
  float4 f4_t_outoforder : packoffset(c13.w); // expected-error {{register or offset bind not valid}} fxc-error {{X3530: register or offset bind c13.w not valid}}
}

sampler myvar_noparens : register; // expected-error {{expected '(' after 'register'}} fxc-error {{X3000: syntax error: unexpected token ';'}}
sampler myvar_noclosebracket: register(ps, s[2); ]; // expected-error {{expected ']'}} expected-error {{expected unqualified-id}} expected-note {{to match this '['}} fxc-error {{X3000: syntax error: unexpected token ')'}}
sampler myvar_norparen: register(ps, s[2]; ); // expected-error {{expected ')'}} expected-error {{expected unqualified-id}} fxc-error {{X3000: syntax error: unexpected token ';'}}
sampler myVar : register(ps_5_0, s);
/*verify-ast
  VarDecl <col:1, col:9> col:9 myVar 'sampler':'SamplerState'
  `-RegisterAssignment <col:17> register(ps_5_0, s0)
*/
sampler myVar2 : register(vs, s[8]);
sampler myVar2_offset : register(vs, s2[8]);
/*verify-ast
  VarDecl <col:1, col:9> col:9 myVar2_offset 'sampler':'SamplerState'
  `-RegisterAssignment <col:25> register(vs, s10)
*/
sampler myVar2_emptyu : register(vs, s2[]); // expected-error {{expected expression}} fxc-error {{X3000: syntax error: unexpected token ']'}}
sampler myVar_2 : register(vs, s8);
// fxc error: error X4017: cannot bind the same variable to multiple constants in the same constant bank
sampler myVar3 : register(ps_5_0, s[0]) : register(vs, s[8]);
// fxc error X3591: incorrect bind semantic
sampler myVar4 : register(vs, t0);          /* fxc-error {{X3591: incorrect bind semantic}} */
sampler myVar65536 : register(vs, s65536);
sampler myVar4294967296 : register(vs, s4294967296); // expected-error {{register number should be an integral numeric string}} fxc-pass {{}}
sampler myVar281474976710656 : register(vs, s281474976710656); // expected-error {{register number should be an integral numeric string}} fxc-pass {{}}
sampler myVar5 : register(vs, s);
/*verify-ast
  VarDecl <col:1, col:9> col:9 myVar5 'sampler':'SamplerState'
  `-RegisterAssignment <col:18> register(vs, s0)
*/
// fxc error X3530: register vs not valid
sampler myVar6 : register(vs); // expected-error {{expected ','}} fxc-error {{X3530: sampler requires an 's' or 't' register}}
// fxc error X3000: syntax error: unexpected token ')'
sampler myVar7 : register(); // expected-error {{expected identifier}} fxc-error {{X3000: syntax error: unexpected token ')'}}
// fxc error X3000: syntax error: unexpected token ';'
sampler myVar8 : ; // expected-error {{expected ';' after top level declarator}} fxc-error {{X3000: syntax error: unexpected token ';'}}
// fxc error X3000: error X3530: register a0 not valid
sampler myVar9 : register(ps, a0); // expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: sampler requires an 's' or 't' register}}
// fxc error X3000: error X3530: register a not valid
sampler myVar10 : register(ps, a); // expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: sampler requires an 's' or 't' register}}
AppendStructuredBuffer<float4> myVar11 : register(ps, u1);
RWStructuredBuffer<float4> myVar11_rw : register(ps, u);
// fxc error X3000: syntax error: unexpected integer constant
sampler myVar12 : register(ps, 3); // expected-error {{expected identifier}} fxc-error {{X3000: syntax error: unexpected integer constant}}
// fxc error X3000: error X3530: register _ not valid
sampler myVar13 : register(ps, _); // expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: sampler requires an 's' or 't' register}}
sampler myVar14 : register(ps, T14);                        /* fxc-error {{X3591: incorrect bind semantic}} */
sampler myVar15 : register(S15);
sampler myVar16 : register(ps, A16); // expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: sampler requires an 's' or 't' register}}

// fxc error X3091: packoffset is only allowed in a constant buffer
sampler myVar17 : packoffset(c0); // expected-error {{packoffset is only allowed in a constant buffer}} fxc-error {{X3091: packoffset is only allowed in a constant buffer}}
sampler myVar_s : register(ps, s);
Texture2D myVar_t : register(ps, t);
Texture2D myVar_t_1 : register(ps, t[1]);
Texture2D myVar_t_1_1 : register(ps, t1[1]),
/*verify-ast
  VarDecl <col:1, col:11> col:11 myVar_t_1_1 'Texture2D':'Texture2D<vector<float, 4> >'
  `-RegisterAssignment <col:25> register(ps, t2)
  VarDecl <col:1, line:167:3> col:3 myVar_t_2_1 'Texture2D':'Texture2D<vector<float, 4> >'
  |-RegisterAssignment <col:17> register(ps, t3)
  `-RegisterAssignment <col:39> register(vs, t0)
*/
  myVar_t_2_1 : register(ps, t2[1]) : register(vs, t0);

// fxc error X3591: incorrect bind semantic
sampler myVar_i : register(ps, i); // expected-error {{invalid register specification, expected 's' or 't' binding}} fxc-error {{X3530: sampler requires an 's' or 't' register}}
float4 myVar_b : register(ps, b);
bool myVar_bool : register(ps, b) : register(ps, c);
/*verify-ast
  VarDecl <col:1, col:6> col:6 myVar_bool 'const bool'
  |-RegisterAssignment <col:19> register(ps, b0)
  `-RegisterAssignment <col:37> register(ps, c0)
*/
// fxc error X3591: incorrect bind semantic
sampler myVar_c : register(ps, c); // expected-error {{invalid register specification, expected 's' or 't' binding}} fxc-error {{X3530: sampler requires an 's' or 't' register}}
// fxc error X3000: error X3530: register z not valid
sampler myVar_z : register(ps, z); // expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: sampler requires an 's' or 't' register}}
sampler myVar_1 : register(ps, s[1]);
sampler myVar_11 : register(ps, s[1+1]);
/*verify-ast
  VarDecl <col:1, col:9> col:9 myVar_11 'sampler':'SamplerState'
  `-RegisterAssignment <col:20> register(ps, s2)
*/
sampler myVar_16 : register(ps, s[15]);
// fxc error X4509: maximum sampler register index exceeded, target has 16 slots, manual bind to slot s1073741823 failed
sampler myVar_n1 : register(ps, s[-1]); // expected-error {{register subcomponent is not an integral constant}} fxc-pass {{}}
sampler myVar_n1p5 : register(ps, s[1.5]);
sampler myVar_s1 : register(ps, s[1], space1);
/*verify-ast
  VarDecl <col:1, col:9> col:9 myVar_s1 'sampler':'SamplerState'
  `-RegisterAssignment <col:20> register(ps, s1, space1)
*/
// fxc error X3591: incorrect bind semantic
sampler myVar_sb : register(ps, s[1], splice); // expected-error {{expected space definition with syntax 'spaceX', where X is an integral value}} fxc-error {{X3591: incorrect bind semantic}}
sampler myVar_sz : register(ps, s[1], spacez); // expected-error {{space number should be an integral numeric string}} fxc-error {{X3591: incorrect bind semantic}}

// Legal in fxc due to compatibility mode:
sampler mySamp_t1 : register(t1);              /* fxc-error {{X3591: incorrect bind semantic}} */

Buffer buff_t2 : register(t2);
Buffer buff_s2 : register(s2);  // expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}}
Buffer buff_u2 : register(u2);  // expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}}
Buffer buff_b2 : register(b2);  // expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}}

Buffer buff_t3 : register(T3);
Buffer buff_s3 : register(S3);  // expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}}
Buffer buff_u3 : register(U3);  // expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}}
Buffer buff_b3 : register(B3);  // expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}}

RasterizerOrderedBuffer<float4> ROVBuff_u1 : register(u1);
RasterizerOrderedBuffer<float4> ROVBuff_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedBuffer<float4> ROVBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedBuffer<float4> ROVBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedBuffer<float4> ROVBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedBuffer<float4> ROVBuff_u2 : register(U2);
RasterizerOrderedBuffer<float4> ROVBuff_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedBuffer<float4> ROVBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedBuffer<float4> ROVBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedBuffer<float4> ROVBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

Texture1D T1D_t1 : register(t1);
Texture1D T1D_u1 : register(u1);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1D T1D_f1 : register(f1);                            /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1D T1D_b1 : register(b1);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1D T1D_s1 : register(s1);                            /* fxc-error {{X3591: incorrect bind semantic}} */

Texture1D T1D_t2 : register(T2);
Texture1D T1D_u2 : register(U2);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1D T1D_f2 : register(F2);                            /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1D T1D_b2 : register(B2);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1D T1D_s2 : register(S2);                            /* fxc-error {{X3591: incorrect bind semantic}} */

Texture1DArray T1DArray_t1 : register(t1);
Texture1DArray T1DArray_u1 : register(u1);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1DArray T1DArray_f1 : register(f1);                  /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1DArray T1DArray_b1 : register(b1);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1DArray T1DArray_s1 : register(s1);                  /* fxc-error {{X3591: incorrect bind semantic}} */

Texture1DArray T1DArray_t2 : register(T2);
Texture1DArray T1DArray_u2 : register(U2);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1DArray T1DArray_f2 : register(F2);                  /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1DArray T1DArray_b2 : register(B2);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture1DArray T1DArray_s2 : register(S2);                  /* fxc-error {{X3591: incorrect bind semantic}} */

Texture2D T2D_t1 : register(t1);
Texture2D T2D_u1 : register(u1);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2D T2D_f1 : register(f1);                            /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2D T2D_b1 : register(b1);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2D T2D_s1 : register(s1);                            /* fxc-error {{X3591: incorrect bind semantic}} */

Texture2D T2D_t2 : register(T2);
Texture2D T2D_u2 : register(U2);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2D T2D_f2 : register(F2);                            /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2D T2D_b2 : register(B2);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2D T2D_s2 : register(S2);                            /* fxc-error {{X3591: incorrect bind semantic}} */

Texture2DMS<float4, 4> T2DMS_t1 : register(t1);
Texture2DMS<float4, 4> T2DMS_u1 : register(u1);             /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DMS<float4, 4> T2DMS_f1 : register(f1);             /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DMS<float4, 4> T2DMS_b1 : register(b1);             /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DMS<float4, 4> T2DMS_s1 : register(s1);             /* fxc-error {{X3591: incorrect bind semantic}} */

Texture2DMS<float4, 4> T2DMS_t2 : register(T2);
Texture2DMS<float4, 4> T2DMS_u2 : register(U2);             /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DMS<float4, 4> T2DMS_f2 : register(F2);             /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DMS<float4, 4> T2DMS_b2 : register(B2);             /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DMS<float4, 4> T2DMS_s2 : register(S2);             /* fxc-error {{X3591: incorrect bind semantic}} */

Texture2DArray T2DArray_t1 : register(t1);
Texture2DArray T2DArray_u1 : register(u1);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DArray T2DArray_f1 : register(f1);                  /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DArray T2DArray_b1 : register(b1);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DArray T2DArray_s1 : register(s1);                  /* fxc-error {{X3591: incorrect bind semantic}} */

Texture2DArray T2DArray_t2 : register(T2);
Texture2DArray T2DArray_u2 : register(U2);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DArray T2DArray_f2 : register(F2);                  /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DArray T2DArray_b2 : register(B2);                  /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture2DArray T2DArray_s2 : register(S2);                  /* fxc-error {{X3591: incorrect bind semantic}} */

Texture3D T3D_t1 : register(t1);
Texture3D T3D_u1 : register(u1);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture3D T3D_f1 : register(f1);                            /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture3D T3D_b1 : register(b1);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture3D T3D_s1 : register(s1);                            /* fxc-error {{X3591: incorrect bind semantic}} */

Texture3D T3D_t2 : register(T2);
Texture3D T3D_u2 : register(U2);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture3D T3D_f2 : register(F2);                            /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture3D T3D_b2 : register(B2);                            /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
Texture3D T3D_s2 : register(S2);                            /* fxc-error {{X3591: incorrect bind semantic}} */

TextureCube TCube_t1 : register(t1);
TextureCube TCube_u1 : register(u1);                        /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCube TCube_f1 : register(f1);                        /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCube TCube_b1 : register(b1);                        /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCube TCube_s1 : register(s1);                        /* fxc-error {{X3591: incorrect bind semantic}} */

TextureCube TCube_t2 : register(T2);
TextureCube TCube_u2 : register(U2);                        /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCube TCube_f2 : register(F2);                        /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCube TCube_b2 : register(B2);                        /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCube TCube_s2 : register(S2);                        /* fxc-error {{X3591: incorrect bind semantic}} */

TextureCubeArray TCubeArray_t1 : register(t1);
TextureCubeArray TCubeArray_u1 : register(u1);              /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCubeArray TCubeArray_f1 : register(f1);              /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCubeArray TCubeArray_b1 : register(b1);              /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCubeArray TCubeArray_s1 : register(s1);              /* fxc-error {{X3591: incorrect bind semantic}} */

TextureCubeArray TCubeArray_t2 : register(T2);
TextureCubeArray TCubeArray_u2 : register(U2);              /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCubeArray TCubeArray_f2 : register(F2);              /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCubeArray TCubeArray_b2 : register(B2);              /* expected-error {{invalid register specification, expected 't' or 's' binding}} fxc-error {{X3530: texture requires a 't' or 's' register}} */
TextureCubeArray TCubeArray_s2 : register(S2);              /* fxc-error {{X3591: incorrect bind semantic}} */

RWTexture1D<float4> RWT1D_u1 : register(u1);
RWTexture1D<float4> RWT1D_t1 : register(t1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1D<float4> RWT1D_f1 : register(f1);                /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1D<float4> RWT1D_b1 : register(b1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1D<float4> RWT1D_s1 : register(s1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture1D<float4> RWT1D_u2 : register(U2);
RWTexture1D<float4> RWT1D_t2 : register(T2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1D<float4> RWT1D_f2 : register(F2);                /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1D<float4> RWT1D_b2 : register(B2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1D<float4> RWT1D_s2 : register(S2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture1DArray<float4> RWT1DArray_u1 : register(u1);
RWTexture1DArray<float4> RWT1DArray_t1 : register(t1);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1DArray<float4> RWT1DArray_f1 : register(f1);      /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1DArray<float4> RWT1DArray_b1 : register(b1);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1DArray<float4> RWT1DArray_s1 : register(s1);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture1DArray<float4> RWT1DArray_u2 : register(U2);
RWTexture1DArray<float4> RWT1DArray_t2 : register(T2);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1DArray<float4> RWT1DArray_f2 : register(F2);      /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1DArray<float4> RWT1DArray_b2 : register(B2);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture1DArray<float4> RWT1DArray_s2 : register(S2);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture2D<float4> RWT2D_u1 : register(u1);
RWTexture2D<float4> RWT2D_t1 : register(t1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2D<float4> RWT2D_f1 : register(f1);                /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2D<float4> RWT2D_b1 : register(b1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2D<float4> RWT2D_s1 : register(s1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture2D<float4> RWT2D_u2 : register(U2);
RWTexture2D<float4> RWT2D_t2 : register(T2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2D<float4> RWT2D_f2 : register(F2);                /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2D<float4> RWT2D_b2 : register(B2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2D<float4> RWT2D_s2 : register(S2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture2DArray<float4> RWT2DArray_u1 : register(u1);
RWTexture2DArray<float4> RWT2DArray_t1 : register(t1);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2DArray<float4> RWT2DArray_f1 : register(f1);      /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2DArray<float4> RWT2DArray_b1 : register(b1);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2DArray<float4> RWT2DArray_s1 : register(s1);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture2DArray<float4> RWT2DArray_u2 : register(U2);
RWTexture2DArray<float4> RWT2DArray_t2 : register(T2);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2DArray<float4> RWT2DArray_f2 : register(F2);      /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2DArray<float4> RWT2DArray_b2 : register(B2);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture2DArray<float4> RWT2DArray_s2 : register(S2);      /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture3D<float4> RWT3D_u1 : register(u1);
RWTexture3D<float4> RWT3D_t1 : register(t1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture3D<float4> RWT3D_f1 : register(f1);                /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture3D<float4> RWT3D_b1 : register(b1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture3D<float4> RWT3D_s1 : register(s1);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWTexture3D<float4> RWT3D_u2 : register(U2);
RWTexture3D<float4> RWT3D_t2 : register(T2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture3D<float4> RWT3D_f2 : register(F2);                /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture3D<float4> RWT3D_b2 : register(B2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWTexture3D<float4> RWT3D_s2 : register(S2);                /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture1D<float4> ROVT1D_u1 : register(u1);
RasterizerOrderedTexture1D<float4> ROVT1D_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1D<float4> ROVT1D_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1D<float4> ROVT1D_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1D<float4> ROVT1D_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture1D<float4> ROVT1D_u2 : register(U2);
RasterizerOrderedTexture1D<float4> ROVT1D_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1D<float4> ROVT1D_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1D<float4> ROVT1D_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1D<float4> ROVT1D_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture1DArray<float4> ROVT1DArray_u1 : register(u1);
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture1DArray<float4> ROVT1DArray_u2 : register(U2);
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture1DArray<float4> ROVT1DArray_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture2D<float4> ROVT2D_u1 : register(u1);
RasterizerOrderedTexture2D<float4> ROVT2D_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2D<float4> ROVT2D_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2D<float4> ROVT2D_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2D<float4> ROVT2D_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture2D<float4> ROVT2D_u2 : register(U2);
RasterizerOrderedTexture2D<float4> ROVT2D_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2D<float4> ROVT2D_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2D<float4> ROVT2D_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2D<float4> ROVT2D_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture2DArray<float4> ROVT2DArray_u1 : register(u1);
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture2DArray<float4> ROVT2DArray_u2 : register(U2);
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture2DArray<float4> ROVT2DArray_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture3D<float4> ROVT3D_u1 : register(u1);
RasterizerOrderedTexture3D<float4> ROVT3D_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture3D<float4> ROVT3D_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture3D<float4> ROVT3D_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture3D<float4> ROVT3D_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedTexture3D<float4> ROVT3D_u2 : register(U2);
RasterizerOrderedTexture3D<float4> ROVT3D_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture3D<float4> ROVT3D_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture3D<float4> ROVT3D_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedTexture3D<float4> ROVT3D_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

ByteAddressBuffer ByteAddressBuff_t1 : register(t1);
ByteAddressBuffer ByteAddressBuff_u1 : register(u1);        /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
ByteAddressBuffer ByteAddressBuff_f1 : register(f1);        /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: buffer requires a 't' register}} */
ByteAddressBuffer ByteAddressBuff_b1 : register(b1);        /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
ByteAddressBuffer ByteAddressBuff_s1 : register(s1);        /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */

ByteAddressBuffer ByteAddressBuff_t2 : register(T2);
ByteAddressBuffer ByteAddressBuff_u2 : register(U2);        /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
ByteAddressBuffer ByteAddressBuff_f2 : register(F2);        /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: buffer requires a 't' register}} */
ByteAddressBuffer ByteAddressBuff_b2 : register(B2);        /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
ByteAddressBuffer ByteAddressBuff_s2 : register(S2);        /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */

RWByteAddressBuffer RWByteAddressBuff_u1 : register(u1);
RWByteAddressBuffer RWByteAddressBuff_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWByteAddressBuffer RWByteAddressBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWByteAddressBuffer RWByteAddressBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWByteAddressBuffer RWByteAddressBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWByteAddressBuffer RWByteAddressBuff_u2 : register(U2);
RWByteAddressBuffer RWByteAddressBuff_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWByteAddressBuffer RWByteAddressBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWByteAddressBuffer RWByteAddressBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWByteAddressBuffer RWByteAddressBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_u1 : register(u1);
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_u2 : register(U2);
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedByteAddressBuffer ROVByteAddressBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

StructuredBuffer<float4> StructuredBuff_t1 : register(t1);
StructuredBuffer<float4> StructuredBuff_u1 : register(u1);    /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
StructuredBuffer<float4> StructuredBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: buffer requires a 't' register}} */
StructuredBuffer<float4> StructuredBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
StructuredBuffer<float4> StructuredBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */

StructuredBuffer<float4> StructuredBuff_t2 : register(T2);
StructuredBuffer<float4> StructuredBuff_u2 : register(U2);    /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
StructuredBuffer<float4> StructuredBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: buffer requires a 't' register}} */
StructuredBuffer<float4> StructuredBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */
StructuredBuffer<float4> StructuredBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 't' binding}} fxc-error {{X3530: buffer requires a 't' register}} */

RWStructuredBuffer<float4> RWStructuredBuff_u1 : register(u1);
RWStructuredBuffer<float4> RWStructuredBuff_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWStructuredBuffer<float4> RWStructuredBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWStructuredBuffer<float4> RWStructuredBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWStructuredBuffer<float4> RWStructuredBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RWStructuredBuffer<float4> RWStructuredBuff_u2 : register(U2);
RWStructuredBuffer<float4> RWStructuredBuff_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWStructuredBuffer<float4> RWStructuredBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWStructuredBuffer<float4> RWStructuredBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RWStructuredBuffer<float4> RWStructuredBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_u1 : register(u1);
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_u2 : register(U2);
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
RasterizerOrderedStructuredBuffer<float4> ROVStructuredBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_u1 : register(u1);
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_u2 : register(U2);
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
ConsumeStructuredBuffer<float4> ConsumeStructuredBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

AppendStructuredBuffer<float4> AppendStructuredBuff_u1 : register(u1);
AppendStructuredBuffer<float4> AppendStructuredBuff_t1 : register(t1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
AppendStructuredBuffer<float4> AppendStructuredBuff_f1 : register(f1);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
AppendStructuredBuffer<float4> AppendStructuredBuff_b1 : register(b1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
AppendStructuredBuffer<float4> AppendStructuredBuff_s1 : register(s1);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

AppendStructuredBuffer<float4> AppendStructuredBuff_u2 : register(U2);
AppendStructuredBuffer<float4> AppendStructuredBuff_t2 : register(T2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
AppendStructuredBuffer<float4> AppendStructuredBuff_f2 : register(F2);    /* expected-error {{register type is unsupported - available types are 'b', 'c', 'i', 's', 't', 'u'}} fxc-error {{X3530: UAV requires a 'u' register}} */
AppendStructuredBuffer<float4> AppendStructuredBuff_b2 : register(B2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */
AppendStructuredBuffer<float4> AppendStructuredBuff_s2 : register(S2);    /* expected-error {{invalid register specification, expected 'u' binding}} fxc-error {{X3530: UAV requires a 'u' register}} */

cbuffer MyBuffer
{
  float4 Element1 : packoffset(c0);
  float1 Element2 : packoffset(c1);
  float1 Element3 : packoffset(c1.y);
  float4 Element4 : packoffset(c10) : packoffset(c10);
  float4 Element5 : packoffset(c10) : MY_SEMANTIC : packoffset(c11), Element6 : packoffset(c12) : MY_SEMANTIC2; // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}}
  /*verify-ast
    VarDecl parent cbuffer <col:3, col:10> col:10 Element5 'const float4':'const vector<float, 4>'
    |-ConstantPacking <col:21> packoffset(c10.x)
    |-SemanticDecl <col:39> "MY_SEMANTIC"
    `-ConstantPacking <col:53> packoffset(c11.x)
    VarDecl parent cbuffer <col:3, col:70> col:70 Element6 'const float4':'const vector<float, 4>'
    |-ConstantPacking <col:81> packoffset(c12.x)
    `-SemanticDecl <col:99> "MY_SEMANTIC2"
  */
}

cbuffer MyBuffer2
{
  float4 Element7 : packoffset(C2);
  float1 Element8 : packoffset(C3);
  float1 Element9 : packoffset(C3.y);
  float4 Element10 : packoffset(C20) : packoffset(C20);
  float4 Element11 : packoffset(C20) : MY_SEMANTIC : packoffset(C21), Element12 : packoffset(C22) : MY_SEMANTIC2; // expected-warning {{packoffset is overridden by another packoffset annotation}} fxc-pass {{}}
  /*verify-ast
    VarDecl parent cbuffer <col:3, col:10> col:10 Element11 'const float4':'const vector<float, 4>'
    |-ConstantPacking <col:22> packoffset(c20.x)
    |-SemanticDecl <col:40> "MY_SEMANTIC"
    `-ConstantPacking <col:54> packoffset(c21.x)
    VarDecl parent cbuffer <col:3, col:71> col:71 Element12 'const float4':'const vector<float, 4>'
    |-ConstantPacking <col:83> packoffset(c22.x)
    `-SemanticDecl <col:101> "MY_SEMANTIC2"
  */
}

Texture2D<float4> Texture : register(t0);
Texture2D<float4> Texture_ : register(t0);
sampler Sampler : register(s0);
// expected-warning@+1{{cannot mix packoffset elements with nonpackoffset elements in a cbuffer}}
cbuffer Parameters : register(b0)
{
  float4   DiffuseColor   : packoffset(c0) : register(c0);
  /*verify-ast
    VarDecl parent cbuffer <col:3, col:12> col:12 DiffuseColor 'const float4':'const vector<float, 4>'
    |-ConstantPacking <col:29> packoffset(c0.x)
    `-RegisterAssignment <col:46> register(c0)
  */
  float4   AlphaTest      : packoffset(c1);
  float3   FogColor       : packoffset(c2);
  float4   FogVector      : packoffset(c3);
  float4x4 WorldViewProj  : packoffset(c4);
  float4x4 WorldViewProj2 : packoffset; // expected-error {{expected '(' after 'packoffset'}} fxc-error {{X3000: syntax error: unexpected token ';'}}
  float4x4 WorldViewProj3 : packoffset(1); // expected-error {{expected identifier}} fxc-error {{X3000: syntax error: unexpected integer constant}}
  float4x4 WorldViewProj4 : packoffset(c4.1);  // expected-error {{expected ')'}} fxc-error {{X3000: syntax error: unexpected float constant}}
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
  HLSLBufferDecl <col:1, line:623:1> line:607:9 cbuffer OuterBuffer
  |-RegisterAssignment <col:23> register(b3)
  |-VarDecl parent cbuffer <line:618:3, col:9> col:9 used OuterItem0 'const float'
  |-HLSLBufferDecl parent <line:619:3, line:621:3> line:619:11 cbuffer InnerBuffer
  | |-RegisterAssignment <col:25> register(b4)
  | `-VarDecl parent cbuffer <line:620:5, col:11> col:11 used InnerItem0 'const float'
  |-EmptyDecl parent cbuffer <line:621:4> col:4
  `-VarDecl parent cbuffer <line:622:3, col:9> col:9 used OuterItem1 'const float'
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
  g_txDiffuse.Sample(myVar281474976710656, float2(1, 2)); // expected-warning {{ignoring return value of function that only reads data}} fxc-pass {{}}
  return 0;
}

[shader("pixel")]
float4 main(float4 param4 : TEXCOORD0) : SV_Target0 {
  float f = OuterItem0 + OuterItem1 + InnerItem0;
  return g_txDiffuse.Sample(myVar_s, float2(1, f));
}