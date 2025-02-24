// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

//////////////////////////////////////////////////////////////////////////////
// Global variables.
float g_foo1 : foo;
float g_foo2 : foo : fubar;
float g_foo3 : foo : fubar : register(c3);
float g_foo4 : register(c4) : foo : fubar;

cbuffer CBInit : register(b2)
{
    float g2_foo5 : foo : fubar : packoffset(c5);
    float g2_foo6 : packoffset(c6) : foo : fubar;
    /*verify-ast
      VarDecl <col:5, col:11> g2_foo6 'float'
      |-ConstantPacking <col:21> packoffset(c6.x)
      |-SemanticDecl <col:38> "foo"
      |-SemanticDecl <col:44> "fubar"
    */
}

cbuffer CBInit2 : register(b3) : cbuffer_semantic           /* fxc-warning {{X3202: usage semantics do not apply to cbuffers}} */
/*verify-ast
  HLSLBufferDecl <col:1, line:36:1> cbuffer CBInit2
  |-RegisterAssignment <col:19> register(b3)
  |-SemanticDecl <col:34> "cbuffer_semantic"
  `-VarDecl <line:35:5, col:11> g3_foo1 'float'
    |-SemanticDecl <col:21> "foo"
*/
{
    float g3_foo1 : foo;
}

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
struct semantic_on_struct {
    float a;
};

struct semantic_on_struct_instance {
/*verify-ast
  CXXRecordDecl <col:1, line:57:1> struct semantic_on_struct_instance definition
  |-CXXRecordDecl <col:1, col:8> struct semantic_on_struct_instance
  `-FieldDecl <line:56:5, col:11> a 'float'
  VarDecl <col:1, line:57:3> g_struct 'struct semantic_on_struct_instance':'semantic_on_struct_instance'
  |-SemanticDecl <col:14> "semantic"
  VarDecl <col:1, line:58:3> g_struct2 'struct semantic_on_struct_instance':'semantic_on_struct_instance'
  |-SemanticDecl <col:15> "semantic2"
*/
    float a;
} g_struct : semantic,
  g_struct2 : semantic2;

//////////////////////////////////////////////////////////////////////////////
// Fields.
struct s_fields {
    float a : semantic_a;
    /*verify-ast
      FieldDecl <col:5, col:11> a 'float'
      |-SemanticDecl <col:15> "semantic_a"
    */
    float b : semantic_b : also_b;
    /*verify-ast
      FieldDecl <col:5, col:11> b 'float'
      |-SemanticDecl <col:15> "semantic_b"
      |-SemanticDecl <col:28> "also_b"
    */
};

//////////////////////////////////////////////////////////////////////////////
// Functions and Parameters
float fn_foo1(float a : a, float b : b) : sem_ret { return 1.0f; }
float fn_foo2(float a : a, float b : b : also_b) : sem_ret : also_ret { return 1.0f; }
/*verify-ast
  FunctionDecl <col:1, col:86> fn_foo2 'float (float, float)'
  |-ParmVarDecl <col:15, col:21> a 'float'
  | |-SemanticDecl <col:25> "a"
  |-ParmVarDecl <col:28, col:34> b 'float'
  | |-SemanticDecl <col:38> "b"
  | |-SemanticDecl <col:42> "also_b"
  `-CompoundStmt <col:71, col:86>
    `-ReturnStmt <col:73, col:80>
      `-FloatingLiteral <col:80> 'float' 1.000000e+00
  `-SemanticDecl <col:52> "sem_ret"
  `-SemanticDecl <col:62> "also_ret"
*/

//////////////////////////////////////////////////////////////////////////////
// Semantics on Resources and Samplers
Texture2D <float> tex : TEXTURE0;
SamplerState samp : SAMPLER0;
cbuffer CBFoo {
    Texture2D <float> tex1 : TEXTURE1;
    SamplerState samp1 : SAMPLER1;
    float cbvalue1;
}
struct semantic_on_resource_fields {
    Texture2D <float> tex : TEXTURE2;
    SamplerState samp : SAMPLER2;
    float value1;
};

// TODO: Fix bug where struct_with_res overlaps bindings of resources in CBFoo
//semantic_on_resource_fields struct_with_res;

ConstantBuffer<semantic_on_resource_fields> CB[1];

//////////////////////////////////////////////////////////////////////////////
// Locals.
float4 main() : SV_Target {

    struct {
        float a : sem_a;
        float b : sem_b : also_b;
    } s;
    struct  anon_semantic {  
        float a : sem_a;
    } s2;

    float f = tex.Sample(samp, float2(0.25,0.5))
        * tex1.Sample(samp1, float2(0.25,0.5)) * cbvalue1
//        * struct_with_res.tex.Sample(struct_with_res.samp, float2(0.5,0.25)) * struct_with_res.value1
        * CB[0].value1;
    return f;
}