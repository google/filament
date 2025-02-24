// RUN: %clang_cc1 -fsyntax-only -ffreestanding -verify %s
// :FXC_VERIFY_ARGUMENTS: /E main /T ps_5_1 /Gec

Texture2D tex : register(t1), tex2 : register(t2)
/*verify-ast
  VarDecl <col:1, col:11> col:11 used tex 'Texture2D':'Texture2D<vector<float, 4> >'
  `-RegisterAssignment <col:17> register(t1)
  VarDecl <col:1, col:31> col:31 tex2 'Texture2D':'Texture2D<vector<float, 4> >'
  `-RegisterAssignment <col:38> register(t2)
  VarDecl <col:1, line:19:10> col:4 texa 'Texture2D [3]'
*/
< int foo=1; >                                              /* expected-warning {{possible effect annotation ignored - effect syntax is deprecated}} fxc-pass {{}} */
{                                                           /* expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
    Texture = tex;
    Filter = MIN_MAG_MIP_LINEAR;
    MaxAnisotropy = 0;
    AddressU = Wrap;
    AddressV = Wrap;
}, texa[3]
<                                                           /* expected-warning {{possible effect annotation ignored - effect syntax is deprecated}} fxc-pass {{}} */
  string Name = "texa";
  int ArraySize = 3;
>;

Texture texCaps;                                            /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */

SamplerState samLinear : register(s7)
/*verify-ast
  VarDecl <col:1, col:14> col:14 used samLinear 'SamplerState'
  `-RegisterAssignment <col:26> register(s7)
*/
< bool foo=1 > 2; >                                         /* expected-warning {{possible effect annotation ignored - effect syntax is deprecated}} fxc-pass {{}} */
{                                                           /* expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
    Texture = tex;
    Filter = MIN_MAG_MIP_LINEAR;
    MaxAnisotropy = 0;
    AddressU = Wrap;
    AddressV = Wrap;
};

// sampler_state objects legal on both SamplerState and SamplerComparisonState objects:
sampler S : register(s1) = sampler_state {texture=tex;};    /* expected-warning {{effect sampler_state assignment ignored - effect syntax is deprecated}} fxc-pass {{}} */
/*verify-ast
  VarDecl <col:1, col:9> col:9 S 'sampler':'SamplerState'
  `-RegisterAssignment <col:13> register(s1)
*/
SamplerComparisonState SC : register(s3) = sampler_state {texture=tex;};    /* expected-warning {{effect sampler_state assignment ignored - effect syntax is deprecated}} fxc-pass {{}} */

float4 main() : SV_Target
{
  //StateBlock SB;                                            /* expected-error {{unknown type name 'StateBlock'}} fxc-error {{X3000: unrecognized identifier 'SB'}} fxc-error {{X3000: unrecognized identifier 'StateBlock'}} */

  {
    int StateBlock = 1;
  }

  // Note this is ok:
  int PixelShadeR = 1;
  RenderTargetView rtv { state=foo; };                      /* expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
  /*verify-ast
    DeclStmt <col:3, col:38>
    `-VarDecl <col:3, col:20> col:20 invalid rtv 'RenderTargetView':'deprecated effect object'
  */
  Texture2D l_tex { state=foo; };                           /* expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
  /*verify-ast
    DeclStmt <col:3, col:33>
    `-VarDecl <col:3, col:13> col:13 l_tex 'Texture2D':'Texture2D<vector<float, 4> >'
  */

  // This is allowed (deprecated effect state block warning):
  int foobar {blah=foo;};                                   /* expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
  // This is not:
  //int foobar2 {blah=foo;} = 5;                              /* expected-error {{expected ';' at end of declaration}} expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-error {{X3000: syntax error: unexpected token '='}} */

  // But this isn't:
  int RenderTargetView = 1;                                 /* fxc-error {{X3000: syntax error: unexpected token 'RenderTargetView'}} */
  int PixelShader = 1;                                      /* fxc-error {{X3000: syntax error: unexpected token 'PixelShader'}} */
  int pixelshader = 1;  // nor is this                      /* fxc-error {{X3000: syntax error: unexpected token 'pixelshader'}} */

  // This is not ok:
  // error X3000: syntax error: unexpected token 'TechNiQue'
  int TechNiQue = 1;                                        /* fxc-error {{X3000: syntax error: unexpected token 'TechNiQue'}} */

  return tex.Sample(samLinear, float2(0.1,0.2));
  /*verify-ast
    ReturnStmt <col:3, col:47>
    `-CXXMemberCallExpr <col:10, col:47> 'vector<float, 4>'
      |-MemberExpr <col:10, col:14> '<bound member function type>' .Sample
      | `-DeclRefExpr <col:10> 'Texture2D':'Texture2D<vector<float, 4> >' lvalue Var 'tex' 'Texture2D':'Texture2D<vector<float, 4> >'
      |-ImplicitCastExpr <col:21> 'SamplerState' <LValueToRValue>
      | `-DeclRefExpr <col:21> 'SamplerState' lvalue Var 'samLinear' 'SamplerState'
      `-CXXFunctionalCastExpr <col:32, col:46> 'float2':'vector<float, 2>' functional cast to float2 <NoOp>
        `-InitListExpr <col:39, col:43> 'float2':'vector<float, 2>'
          |-ImplicitCastExpr <col:39> 'float' <FloatingCast>
          | `-FloatingLiteral <col:39> 'literal float' 1.000000e-01
          `-ImplicitCastExpr <col:43> 'float' <FloatingCast>
            `-FloatingLiteral <col:43> 'literal float' 2.000000e-01
  */
}

// Effects objects to ignore:
// These all accept/ignore effect annotations < ... >
// These all accept/ignore state block syntax
texture tex1 < int foo=1; > { state=foo; };   // Case insensitive!    /* expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{effect state block ignored - effect syntax is deprecated}} expected-warning {{possible effect annotation ignored - effect syntax is deprecated}} fxc-pass {{}} */
static const PixelShader ps1 { state=foo; };                /* expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
/*verify-ast
  VarDecl <col:1, col:26> col:26 invalid ps1 'const PixelShader':'const deprecated effect object' static
*/
// expected-note@? {{'PixelShader' declared here}}
//PixelShadeR ps < int foo=1;>  = ps1;   // Case insensitive! /* expected-error {{unknown type name 'PixelShadeR'; did you mean 'PixelShader'?}} expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{possible effect annotation ignored - effect syntax is deprecated}} expected-error {{use of undeclared identifier 'ps1'}}fxc-pass {{}} */
/*verify-ast
  VarDecl <col:1, col:13> col:13 invalid ps 'PixelShader':'deprecated effect object'
*/
// expected-note@? {{'VertexShader' declared here}}
//VertexShadeR vs;        // Case insensitive!                /* expected-error {{unknown type name 'VertexShadeR'; did you mean 'VertexShader'?}} expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */

// Case sensitive
pixelfragment pfrag;                                        /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */
vertexfragment vfrag;                                       /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */
ComputeShader cs;                                           /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */
DomainShader ds;                                            /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */
GeometryShader gs;                                          /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */
HullShader hs;                                              /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */
BlendState BS { state=foo; };                               /* expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
DepthStencilState DSS { state=foo; };                       /* expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
DepthStencilView SDV;                                       /* expected-warning {{effect object ignored - effect syntax is deprecated}} fxc-pass {{}} */
RasterizerState RS { state=foo; };                          /* expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
RenderTargetView RTV < int foo=1;> ;                        /* expected-warning {{effect object ignored - effect syntax is deprecated}} expected-warning {{possible effect annotation ignored - effect syntax is deprecated}} fxc-pass {{}} */

// Not case-sensitive!  Identifier optional
technique T0                                                /* expected-warning {{effect technique ignored - effect syntax is deprecated}} fxc-pass {{}} */
/*verify-ast
  No matching AST found for line!
*/
{
  pass {}
}
Technique                                                   /* expected-warning {{effect technique ignored - effect syntax is deprecated}} fxc-pass {{}} */
{
  pass {}
}

// Case-sensitive:    Identifier optional
technique10 T10                                             /* expected-warning {{effect technique ignored - effect syntax is deprecated}} fxc-pass {{}} */
/*verify-ast
  No matching AST found for line!
*/
{
  pass {}
}

technique10                                                 /* expected-warning {{effect technique ignored - effect syntax is deprecated}} fxc-pass {{}} */
{
  pass {}
}

// We don't bother handling weird casing, so this will be a syntax error:
//TechNiQue                                                   /* expected-error {{HLSL requires a type specifier for all declarations}} fxc-pass {{}} */
/*verify-ast
  VarDecl <col:1> col:1 invalid TechNiQue 'int'
*/
//{                                                           /* expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
//  pass {}
//}                                                           /* expected-error {{expected ';' after top level declarator}} fxc-pass {{}} */
int foobar3;
/*verify-ast
  VarDecl <col:1, col:5> col:5 foobar3 'int'
*/
//TechNique T5                                                /* expected-error {{unknown type name 'TechNique'}} fxc-pass {{}} */
/*verify-ast
  VarDecl <col:1, col:11> col:11 invalid T5 'int'
*/
//{                                                           /* expected-warning {{effect state block ignored - effect syntax is deprecated}} fxc-pass {{}} */
//  pass {}
//}                                                           /* expected-error {{expected ';' after top level declarator}} fxc-pass {{}} */
int foobar4;
/*verify-ast
  VarDecl <col:1, col:5> col:5 foobar4 'int'
*/