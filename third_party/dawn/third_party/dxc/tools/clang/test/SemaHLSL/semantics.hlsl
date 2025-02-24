// RUN: %dxc -Tlib_6_3 -verify %s

//////////////////////////////////////////////////////////////////////////////
// Global variables.
float g_foo1 : foo;
float g_foo2 : foo : fubar;
float g_foo3 : foo : fubar : register(c3);
float g_foo4 : register(c4) : foo : fubar;
// expected-warning@+1{{cannot mix packoffset elements with nonpackoffset elements in a cbuffer}}
cbuffer CBInit : register(b2)
{
    float g2_foo1 : foo;                                    /* fxc-error {{X3530: cannot mix packoffset elements with nonpackoffset elements in a cbuffer}} */
    float g2_foo2 : foo : fubar;                            /* fxc-error {{X3530: cannot mix packoffset elements with nonpackoffset elements in a cbuffer}} */
    float g2_foo3 : foo : fubar : register(c3);             /* fxc-error {{X3530: cannot mix packoffset elements with nonpackoffset elements in a cbuffer}} */
    float g2_foo4 : register(c4) : foo : fubar;             /* fxc-error {{X3530: cannot mix packoffset elements with nonpackoffset elements in a cbuffer}} */
    float g2_foo5 : foo : fubar : packoffset(c5);
    float g2_foo6 : packoffset(c6) : foo : fubar;
    /*verify-ast
      VarDecl parent cbuffer <col:5, col:11> col:11 g2_foo6 'const float'
      |-ConstantPacking <col:21> packoffset(c6.x)
      |-SemanticDecl <col:38> "foo"
      `-SemanticDecl <col:44> "fubar"
    */
}

cbuffer CBInit2 : register(b3) : cbuffer_semantic           /* fxc-warning {{X3202: usage semantics do not apply to cbuffers}} */
/*verify-ast
  HLSLBufferDecl <col:1, line:36:1> line:26:9 cbuffer CBInit2
  |-RegisterAssignment <col:19> register(b3)
  |-SemanticDecl <col:34> "cbuffer_semantic"
  `-VarDecl parent cbuffer <line:35:5, col:11> col:11 g3_foo1 'const float'
    `-SemanticDecl <col:21> "foo"
*/
{
    float g3_foo1 : foo;
}

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
typedef float t_f : SEMANTIC;                               /* expected-error {{semantic is not a valid modifier for a typedef}} fxc-error {{X3000: syntax error: unexpected token ':'}} */

struct semantic_on_struct : semantic {                      /* expected-error {{expected class name}} fxc-error {{X3000: syntax error: unexpected token 'semantic'}} */
    float a;
};

struct semantic_on_struct_instance {
/*verify-ast
  CXXRecordDecl <col:1, line:57:1> line:46:8 struct semantic_on_struct_instance definition
  |-CXXRecordDecl <col:1, col:8> col:8 implicit struct semantic_on_struct_instance
  `-FieldDecl <line:56:5, col:11> col:11 a 'float'
  VarDecl <col:1, line:57:3> col:3 g_struct 'const struct semantic_on_struct_instance':'const semantic_on_struct_instance'
  `-SemanticDecl <col:14> "semantic"
  VarDecl <col:1, line:58:3> col:3 g_struct2 'const struct semantic_on_struct_instance':'const semantic_on_struct_instance'
  `-SemanticDecl <col:15> "semantic2"
*/
    float a;
} g_struct : semantic,
  g_struct2 : semantic2;

//////////////////////////////////////////////////////////////////////////////
// Fields.
struct s_fields {
    float a : semantic_a;
    /*verify-ast
      FieldDecl <col:5, col:11> col:11 a 'float'
      `-SemanticDecl <col:15> "semantic_a"
    */
    float b : semantic_b : also_b;
    /*verify-ast
      FieldDecl <col:5, col:11> col:11 b 'float'
      |-SemanticDecl <col:15> "semantic_b"
      `-SemanticDecl <col:28> "also_b"
    */
};

//////////////////////////////////////////////////////////////////////////////
// Functions and Parameters
float fn_foo1(float a : a, float b : b) : sem_ret { return 1.0f; }
float fn_foo2(float a : a, float b : b : also_b) : sem_ret : also_ret { return 1.0f; }
/*verify-ast
  FunctionDecl <col:1, col:86> col:7 fn_foo2 'float (float, float)'
  |-ParmVarDecl <col:15, col:21> col:21 a 'float'
  | `-SemanticDecl <col:25> "a"
  |-ParmVarDecl <col:28, col:34> col:34 b 'float'
  | |-SemanticDecl <col:38> "b"
  | `-SemanticDecl <col:42> "also_b"
  |-CompoundStmt <col:71, col:86>
  | `-ReturnStmt <col:73, col:80>
  |   `-FloatingLiteral <col:80> 'float' 1.000000e+00
  |-SemanticDecl <col:52> "sem_ret"
  `-SemanticDecl <col:62> "also_ret"
*/

//////////////////////////////////////////////////////////////////////////////
// Locals.
void vain() {
    float a : Sem_a;                                        /* expected-error {{semantic is not a valid modifier for a local variable}} fxc-error {{X3043: 'a': local variables cannot have semantics}} */
    float b : Sem_b : also_b;                               /* expected-error {{semantic is not a valid modifier for a local variable}} expected-error {{semantic is not a valid modifier for a local variable}} fxc-error {{X3043: 'b': local variables cannot have semantics}} */
    float a2 : Sem_a2 = 1.0f;                               /* expected-error {{semantic is not a valid modifier for a local variable}} fxc-error {{X3043: 'a2': local variables cannot have semantics}} */
    float b2 : Sem_b2 : also_b2 = 2.0f;                     /* expected-error {{semantic is not a valid modifier for a local variable}} expected-error {{semantic is not a valid modifier for a local variable}} fxc-error {{X3043: 'b2': local variables cannot have semantics}} */

    struct {
        float a : sem_a;
        float b : sem_b : also_b;
    } s;
    struct : anon_semantic {                                /* expected-error {{expected class name}} fxc-error {{X3000: syntax error: unexpected token ':'}} */
        float a : sem_a;
    } s2 : semantic;                                        /* expected-error {{semantic is not a valid modifier for a local variable}} fxc-pass {{}} */
}