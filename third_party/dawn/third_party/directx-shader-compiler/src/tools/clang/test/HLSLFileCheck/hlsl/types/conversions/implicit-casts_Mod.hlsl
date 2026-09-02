// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T ps_5_1 implicit-casts.hlsl

// without also putting them in a static assertion

// __decltype is the GCC way of saying 'decltype', but doesn't require C++11
#define VERIFY_FXC
#ifdef VERIFY_FXC
#define VERIFY_TYPES(typ, exp) {typ _tmp_var_ = exp;}
#else
#endif

// The following is meant to be processed by the CodeTags extension in the "VS For Everything" Visual Studio extension:
/*<py>
import re
rxComments = re.compile(r'(//.*|/\*.*?\*\/)')
def strip_comments(line):
    line = rxComments.sub('', line)
    return line.strip()
    saved = {}
    for line in lines:
        key = strip_comments(line)
        if key and line.strip() != key:
            saved[key] = line
    return saved
    return [saved.get(line.strip(), line) for line in lines]
def modify(lines, newlines):

cmp_types = [ # list of (type, shorthand)
  ('float', 'f'),
  ('int', 'i'),
  ('uint', 'u'),
  ('bool', 'b'),
  ('double', 'd'),
  ('min16float', 'm16f'),
  ]
vec_dims = [1, 2, 4]
mat_dims = [(1,1), (4,1), (1,4), (4,4)]
def scalar_types(cmp_types = cmp_types):
  return [{'type': cmp, 'id': id, 'val': (n+1)*100}
          for n, (cmp, id) in enumerate(cmp_types)]
def vector_types(cmp_types, vec_dims = vec_dims):
  return [{'type': cmp+str(d), 'id': id+str(d), 'val': (n+1)*100 + d}
          for n, (cmp, id) in enumerate(cmp_types)
          for d in vec_dims]
def matrix_types(cmp_types, mat_dims = mat_dims):
  return [{'type': '%s%dx%d' % (cmp, d1, d2), 'id': '%s%dx%d' % (id, d1, d2), 'val': (n+1)*100 + d1*10 + d2}
          for n, (cmp, id) in enumerate(cmp_types)
          for d1, d2 in mat_dims]
def all_types(cmp_types = cmp_types, vec_dims = vec_dims, mat_dims = mat_dims):
  return scalar_types(cmp_types) + vector_types(cmp_types, vec_dims) + matrix_types(cmp_types, mat_dims)
def gen_code(template, combos = all_types()):
  return [template % combo for combo in combos]

# For overloaded functions, scalar, vector1 and matrix1x1 are ambiguous,
# so cut the single component vector/matrix out:
overload_types = all_types(cmp_types=cmp_types[:-1], vec_dims=vec_dims[1:], mat_dims=mat_dims[1:])
</py>
*/

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(type)s g_%(id)s;'))</py>
// GENERATED_CODE:BEGIN
float g_f;
int g_i;
uint g_u;
bool g_b;
double g_d;
min16float g_m16f;
float1 g_f1;
float2 g_f2;
float4 g_f4;
int1 g_i1;
int2 g_i2;
int4 g_i4;
uint1 g_u1;
uint2 g_u2;
uint4 g_u4;
bool1 g_b1;
bool2 g_b2;
bool4 g_b4;
double1 g_d1;
double2 g_d2;
double4 g_d4;
min16float1 g_m16f1;
min16float2 g_m16f2;
min16float4 g_m16f4;
float1x1 g_f1x1;
float4x1 g_f4x1;
float1x4 g_f1x4;
float4x4 g_f4x4;
int1x1 g_i1x1;
int4x1 g_i4x1;
int1x4 g_i1x4;
int4x4 g_i4x4;
uint1x1 g_u1x1;
uint4x1 g_u4x1;
uint1x4 g_u1x4;
uint4x4 g_u4x4;
bool1x1 g_b1x1;
bool4x1 g_b4x1;
bool1x4 g_b1x4;
bool4x4 g_b4x4;
double1x1 g_d1x1;
double4x1 g_d4x1;
double1x4 g_d1x4;
double4x4 g_d4x4;
min16float1x1 g_m16f1x1;
min16float4x1 g_m16f4x1;
min16float1x4 g_m16f1x4;
min16float4x4 g_m16f4x4;
// GENERATED_CODE:END

//min16float overload1(min16float v) { return (min16float)600; }
//  function return value cannot have __fp16 type; did you forget * ?
//  parameters cannot have __fp16 type; did you forget * ?

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(type)s overload1(%(type)s v) { return (%(type)s)%(val)s; }', overload_types))</py>
// GENERATED_CODE:BEGIN
float overload1(float v) { return (float)100; }
int overload1(int v) { return (int)200; }
uint overload1(uint v) { return (uint)300; }
bool overload1(bool v) { return (bool)400; }
double overload1(double v) { return (double)500; }
float2 overload1(float2 v) { return (float2)102; }
float4 overload1(float4 v) { return (float4)104; }
int2 overload1(int2 v) { return (int2)202; }
int4 overload1(int4 v) { return (int4)204; }
uint2 overload1(uint2 v) { return (uint2)302; }
uint4 overload1(uint4 v) { return (uint4)304; }
bool2 overload1(bool2 v) { return (bool2)402; }
bool4 overload1(bool4 v) { return (bool4)404; }
double2 overload1(double2 v) { return (double2)502; }
double4 overload1(double4 v) { return (double4)504; }
float4x1 overload1(float4x1 v) { return (float4x1)141; }
float1x4 overload1(float1x4 v) { return (float1x4)114; }
float4x4 overload1(float4x4 v) { return (float4x4)144; }
int4x1 overload1(int4x1 v) { return (int4x1)241; }
int1x4 overload1(int1x4 v) { return (int1x4)214; }
int4x4 overload1(int4x4 v) { return (int4x4)244; }
uint4x1 overload1(uint4x1 v) { return (uint4x1)341; }
uint1x4 overload1(uint1x4 v) { return (uint1x4)314; }
uint4x4 overload1(uint4x4 v) { return (uint4x4)344; }
bool4x1 overload1(bool4x1 v) { return (bool4x1)441; }
bool1x4 overload1(bool1x4 v) { return (bool1x4)414; }
bool4x4 overload1(bool4x4 v) { return (bool4x4)444; }
double4x1 overload1(double4x1 v) { return (double4x1)541; }
double1x4 overload1(double1x4 v) { return (double1x4)514; }
double4x4 overload1(double4x4 v) { return (double4x4)544; }
// GENERATED_CODE:END

// <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(type)s overload2(%(type)s v1, %(type)s v2) { return (%(type)s)%(val)s; }', overload_types))</py>
// GENERATED_CODE:BEGIN
float overload2(float v1, float v2) { return (float)100; }
int overload2(int v1, int v2) { return (int)200; }
uint overload2(uint v1, uint v2) { return (uint)300; }
bool overload2(bool v1, bool v2) { return (bool)400; }
double overload2(double v1, double v2) { return (double)500; }
float2 overload2(float2 v1, float2 v2) { return (float2)102; }
float4 overload2(float4 v1, float4 v2) { return (float4)104; }
int2 overload2(int2 v1, int2 v2) { return (int2)202; }
int4 overload2(int4 v1, int4 v2) { return (int4)204; }
uint2 overload2(uint2 v1, uint2 v2) { return (uint2)302; }
uint4 overload2(uint4 v1, uint4 v2) { return (uint4)304; }
bool2 overload2(bool2 v1, bool2 v2) { return (bool2)402; }
bool4 overload2(bool4 v1, bool4 v2) { return (bool4)404; }
double2 overload2(double2 v1, double2 v2) { return (double2)502; }
double4 overload2(double4 v1, double4 v2) { return (double4)504; }
float4x1 overload2(float4x1 v1, float4x1 v2) { return (float4x1)141; }
float1x4 overload2(float1x4 v1, float1x4 v2) { return (float1x4)114; }
float4x4 overload2(float4x4 v1, float4x4 v2) { return (float4x4)144; }
int4x1 overload2(int4x1 v1, int4x1 v2) { return (int4x1)241; }
int1x4 overload2(int1x4 v1, int1x4 v2) { return (int1x4)214; }
int4x4 overload2(int4x4 v1, int4x4 v2) { return (int4x4)244; }
uint4x1 overload2(uint4x1 v1, uint4x1 v2) { return (uint4x1)341; }
uint1x4 overload2(uint1x4 v1, uint1x4 v2) { return (uint1x4)314; }
uint4x4 overload2(uint4x4 v1, uint4x4 v2) { return (uint4x4)344; }
bool4x1 overload2(bool4x1 v1, bool4x1 v2) { return (bool4x1)441; }
bool1x4 overload2(bool1x4 v1, bool1x4 v2) { return (bool1x4)414; }
bool4x4 overload2(bool4x4 v1, bool4x4 v2) { return (bool4x4)444; }
double4x1 overload2(double4x1 v1, double4x1 v2) { return (double4x1)541; }
double1x4 overload2(double1x4 v1, double1x4 v2) { return (double1x4)514; }
double4x4 overload2(double4x4 v1, double4x4 v2) { return (double4x4)544; }
// GENERATED_CODE:END

float test() {
  // <py::lines('GENERATED_CODE')>modify(lines, gen_code('%(type)s %(id)s = g_%(id)s;'))</py>
  // GENERATED_CODE:BEGIN
  float f = g_f;
  int i = g_i;
  uint u = g_u;
  bool b = g_b;
  double d = g_d;
  min16float m16f = g_m16f;
  float1 f1 = g_f1;
  float2 f2 = g_f2;
  float4 f4 = g_f4;
  int1 i1 = g_i1;
  int2 i2 = g_i2;
  int4 i4 = g_i4;
  uint1 u1 = g_u1;
  uint2 u2 = g_u2;
  uint4 u4 = g_u4;
  bool1 b1 = g_b1;
  bool2 b2 = g_b2;
  bool4 b4 = g_b4;
  double1 d1 = g_d1;
  double2 d2 = g_d2;
  double4 d4 = g_d4;
  min16float1 m16f1 = g_m16f1;
  min16float2 m16f2 = g_m16f2;
  min16float4 m16f4 = g_m16f4;
  float1x1 f1x1 = g_f1x1;
  float4x1 f4x1 = g_f4x1;
  float1x4 f1x4 = g_f1x4;
  float4x4 f4x4 = g_f4x4;
  int1x1 i1x1 = g_i1x1;
  int4x1 i4x1 = g_i4x1;
  int1x4 i1x4 = g_i1x4;
  int4x4 i4x4 = g_i4x4;
  uint1x1 u1x1 = g_u1x1;
  uint4x1 u4x1 = g_u4x1;
  uint1x4 u1x4 = g_u1x4;
  uint4x4 u4x4 = g_u4x4;
  bool1x1 b1x1 = g_b1x1;
  bool4x1 b4x1 = g_b4x1;
  bool1x4 b1x4 = g_b1x4;
  bool4x4 b4x4 = g_b4x4;
  double1x1 d1x1 = g_d1x1;
  double4x1 d4x1 = g_d4x1;
  double1x4 d1x4 = g_d1x4;
  double4x4 d4x4 = g_d4x4;
  min16float1x1 m16f1x1 = g_m16f1x1;
  min16float4x1 m16f4x1 = g_m16f4x1;
  min16float1x4 m16f1x4 = g_m16f1x4;
  min16float4x4 m16f4x4 = g_m16f4x4;
  // GENERATED_CODE:END

  float3  f3 = f4;                                          /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */
  int3x1 i3x1 = i4x4;                                       /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */
  /*verify-ast
    DeclStmt <col:3, col:21>
    `-VarDecl <col:3, col:17> i3x1 'int3x1':'matrix<int, 3, 1>'
      `-ImplicitCastExpr <col:17> 'matrix<int, 3, 1>':'matrix<int, 3, 1>' <HLSLMatrixTruncationCast>
        `-ImplicitCastExpr <col:17> 'int4x4':'matrix<int, 4, 4>' <LValueToRValue>
          `-DeclRefExpr <col:17> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
  */

  VERIFY_TYPES(float, f * f1);
  VERIFY_TYPES(float4, f * f4);
  VERIFY_TYPES(float4, f1 * f4);
  VERIFY_TYPES(float4, i4 * f1);
  VERIFY_TYPES(float4x4, i4x4 * f);
  VERIFY_TYPES(float4x4, f * i4x4);
  VERIFY_TYPES(bool, b = i4);                   /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */

  VERIFY_TYPES(float4x4, overload1(i4x4 * f));
  VERIFY_TYPES(float4x4, overload1(i4x4 * 1.5F));
  VERIFY_TYPES(float4x4, overload1(i4x4 * 1.5));
  VERIFY_TYPES(double2, overload1(i2 * 1.5L));
  VERIFY_TYPES(float4x4, overload1(f4x4 * 2));

  // TODO: Should there be a narrowing warning here due to implicit cast of float to int type?
  VERIFY_TYPES(int4x4, overload2(i4x4, f));
  VERIFY_TYPES(int4x4, overload2(f, i4x4));

  // ambiguous:
  //VERIFY_TYPES(float4, overload2(f4, i4));

  VERIFY_TYPES(float, overload2(f, 1.0));
  VERIFY_TYPES(float, overload2(1.0, f));
  VERIFY_TYPES(double, overload2(d, 1.0));
  VERIFY_TYPES(double, overload2(1.0, d));

  i4 = i;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'int4':'vector<int, 4>' '='
    |-DeclRefExpr <col:3> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
    `-ImplicitCastExpr <col:8> 'vector<int, 4>':'vector<int, 4>' <HLSLVectorSplat>
      `-ImplicitCastExpr <col:8> 'int' <LValueToRValue>
        `-DeclRefExpr <col:8> 'int' lvalue Var 'i' 'int'
  */
  i4 = i1x4;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'int4':'vector<int, 4>' '='
    |-DeclRefExpr <col:3> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
    `-ImplicitCastExpr <col:8> 'vector<int, 4>':'vector<int, 4>' <HLSLMatrixToVectorCast>
      `-ImplicitCastExpr <col:8> 'int1x4':'matrix<int, 1, 4>' <LValueToRValue>
        `-DeclRefExpr <col:8> 'int1x4':'matrix<int, 1, 4>' lvalue Var 'i1x4' 'int1x4':'matrix<int, 1, 4>'
  */
  i4 = i4x1;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'int4':'vector<int, 4>' '='
    |-DeclRefExpr <col:3> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
    `-ImplicitCastExpr <col:8> 'vector<int, 4>':'vector<int, 4>' <HLSLMatrixToVectorCast>
      `-ImplicitCastExpr <col:8> 'int4x1':'matrix<int, 4, 1>' <LValueToRValue>
        `-DeclRefExpr <col:8> 'int4x1':'matrix<int, 4, 1>' lvalue Var 'i4x1' 'int4x1':'matrix<int, 4, 1>'
  */
  i4x4 = i;
  /*verify-ast
    BinaryOperator <col:3, col:10> 'int4x4':'matrix<int, 4, 4>' '='
    |-DeclRefExpr <col:3> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
    `-ImplicitCastExpr <col:10> 'matrix<int, 4, 4>':'matrix<int, 4, 4>' <HLSLMatrixSplat>
      `-ImplicitCastExpr <col:10> 'int' <LValueToRValue>
        `-DeclRefExpr <col:10> 'int' lvalue Var 'i' 'int'
  */
  i = i4x4;                                     /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */
  /*verify-ast
    BinaryOperator <col:3, col:7> 'int' '='
    |-DeclRefExpr <col:3> 'int' lvalue Var 'i' 'int'
    `-ImplicitCastExpr <col:7> 'int' <HLSLMatrixToScalarCast>
      `-ImplicitCastExpr <col:7> 'matrix<int, 1, 1>':'matrix<int, 1, 1>' <HLSLMatrixTruncationCast>
        `-ImplicitCastExpr <col:7> 'int4x4':'matrix<int, 4, 4>' <LValueToRValue>
          `-DeclRefExpr <col:7> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
  */
  /*verify-ast
    BinaryOperator <col:3, col:10> 'int4x4':'matrix<int, 4, 4>' '='
    |-DeclRefExpr <col:3> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
    `-DeclRefExpr <col:10> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
  */
  /*verify-ast
    BinaryOperator <col:3, col:10> 'int4x4':'matrix<int, 4, 4>' '='
    |-DeclRefExpr <col:3> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
    `-DeclRefExpr <col:10> 'int4x1':'matrix<int, 4, 1>' lvalue Var 'i4x1' 'int4x1':'matrix<int, 4, 1>'
  */
  /*verify-ast
    BinaryOperator <col:3, col:10> 'int4x4':'matrix<int, 4, 4>' '='
    |-DeclRefExpr <col:3> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
    `-DeclRefExpr <col:10> 'int1x4':'matrix<int, 1, 4>' lvalue Var 'i1x4' 'int1x4':'matrix<int, 1, 4>'
  */
  i4 = i1;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'int4':'vector<int, 4>' '='
    |-DeclRefExpr <col:3> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
    `-ImplicitCastExpr <col:8> 'vector<int, 4>':'vector<int, 4>' <HLSLVectorSplat>
      `-ImplicitCastExpr <col:8> 'int' <HLSLVectorToScalarCast>
        `-ImplicitCastExpr <col:8> 'int1':'vector<int, 1>' <LValueToRValue>
          `-DeclRefExpr <col:8> 'int1':'vector<int, 1>' lvalue Var 'i1' 'int1':'vector<int, 1>'
  */

  b = i;
  /*verify-ast
    BinaryOperator <col:3, col:7> 'bool' '='
    |-DeclRefExpr <col:3> 'bool' lvalue Var 'b' 'bool'
    `-ImplicitCastExpr <col:7> 'bool' <IntegralToBoolean>
      `-ImplicitCastExpr <col:7> 'int' <LValueToRValue>
        `-DeclRefExpr <col:7> 'int' lvalue Var 'i' 'int'
  */
  b1 = i1;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'bool1':'vector<bool, 1>' '='
    |-DeclRefExpr <col:3> 'bool1':'vector<bool, 1>' lvalue Var 'b1' 'bool1':'vector<bool, 1>'
    `-ImplicitCastExpr <col:8> 'vector<bool, 1>' <HLSLCC_IntegralToBoolean>
      `-ImplicitCastExpr <col:8> 'int1':'vector<int, 1>' <LValueToRValue>
        `-DeclRefExpr <col:8> 'int1':'vector<int, 1>' lvalue Var 'i1' 'int1':'vector<int, 1>'
  */
  b4 = i1;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'bool4':'vector<bool, 4>' '='
    |-DeclRefExpr <col:3> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
    `-ImplicitCastExpr <col:8> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
      `-ImplicitCastExpr <col:8> 'bool' <IntegralToBoolean>
        `-ImplicitCastExpr <col:8> 'int' <HLSLVectorToScalarCast>
          `-ImplicitCastExpr <col:8> 'int1':'vector<int, 1>' <LValueToRValue>
            `-DeclRefExpr <col:8> 'int1':'vector<int, 1>' lvalue Var 'i1' 'int1':'vector<int, 1>'
  */
  b4 = i4;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'bool4':'vector<bool, 4>' '='
    |-DeclRefExpr <col:3> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
    `-ImplicitCastExpr <col:8> 'vector<bool, 4>' <HLSLCC_IntegralToBoolean>
      `-ImplicitCastExpr <col:8> 'int4':'vector<int, 4>' <LValueToRValue>
        `-DeclRefExpr <col:8> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
  */
  b4x4 = i4x4;
  /*verify-ast
    BinaryOperator <col:3, col:10> 'bool4x4':'matrix<bool, 4, 4>' '='
    |-DeclRefExpr <col:3> 'bool4x4':'matrix<bool, 4, 4>' lvalue Var 'b4x4' 'bool4x4':'matrix<bool, 4, 4>'
    `-ImplicitCastExpr <col:10> 'matrix<bool, 4, 4>' <HLSLCC_IntegralToBoolean>
      `-ImplicitCastExpr <col:10> 'int4x4':'matrix<int, 4, 4>' <LValueToRValue>
        `-DeclRefExpr <col:10> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
  */
  /*verify-ast
    BinaryOperator <col:3, col:8> 'int4':'vector<int, 4>' '='
    |-DeclRefExpr <col:3> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
    `-DeclRefExpr <col:8> 'bool4x4':'matrix<bool, 4, 4>' lvalue Var 'b4x4' 'bool4x4':'matrix<bool, 4, 4>'
  */
  /*verify-ast
    BinaryOperator <col:3, col:10> 'int4x1':'matrix<int, 4, 1>' '='
    |-DeclRefExpr <col:3> 'int4x1':'matrix<int, 4, 1>' lvalue Var 'i4x1' 'int4x1':'matrix<int, 4, 1>'
    `-DeclRefExpr <col:10> 'bool1x4':'matrix<bool, 1, 4>' lvalue Var 'b1x4' 'bool1x4':'matrix<bool, 1, 4>'
  */
  f = b;
  /*verify-ast
    BinaryOperator <col:3, col:7> 'float' '='
    |-DeclRefExpr <col:3> 'float' lvalue Var 'f' 'float'
    `-ImplicitCastExpr <col:7> 'bool' <LValueToRValue>
      `-DeclRefExpr <col:7> 'bool' lvalue Var 'b' 'bool'
  */
  f = b4;                                       /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */
  /*verify-ast
    BinaryOperator <col:3, col:7> 'float' '='
    |-DeclRefExpr <col:3> 'float' lvalue Var 'f' 'float'
    `-ImplicitCastExpr <col:7> 'bool' <HLSLVectorToScalarCast>
      `-ImplicitCastExpr <col:7> 'vector<bool, 1>':'vector<bool, 1>' <HLSLVectorTruncationCast>
        `-ImplicitCastExpr <col:7> 'bool4':'vector<bool, 4>' <LValueToRValue>
          `-DeclRefExpr <col:7> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
  */
  f = i;
  /*verify-ast
    BinaryOperator <col:3, col:7> 'float' '='
    |-DeclRefExpr <col:3> 'float' lvalue Var 'f' 'float'
    `-ImplicitCastExpr <col:7> 'float' <IntegralToFloating>
      `-ImplicitCastExpr <col:7> 'int' <LValueToRValue>
        `-DeclRefExpr <col:7> 'int' lvalue Var 'i' 'int'
  */
  f4 = i4;
  /*verify-ast
    BinaryOperator <col:3, col:8> 'float4':'vector<float, 4>' '='
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-ImplicitCastExpr <col:8> 'vector<float, 4>' <HLSLCC_IntegralToFloating>
      `-ImplicitCastExpr <col:8> 'int4':'vector<int, 4>' <LValueToRValue>
        `-DeclRefExpr <col:8> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
  */

  f = i * 0.5;
  /*verify-ast
    BinaryOperator <col:3, col:11> 'float' '='
    |-DeclRefExpr <col:3> 'float' lvalue Var 'f' 'float'
    `-BinaryOperator <col:7, col:11> 'float' '*'
      |-ImplicitCastExpr <col:7> 'float' <IntegralToFloating>
      | `-ImplicitCastExpr <col:7> 'int' <LValueToRValue>
      |   `-DeclRefExpr <col:7> 'int' lvalue Var 'i' 'int'
      `-ImplicitCastExpr <col:11> 'float' <FloatingCast>
        `-FloatingLiteral <col:11> 'literal float' 5.000000e-01
  */
  f4 = i4 * 0.5;
  /*verify-ast
    BinaryOperator <col:3, col:13> 'float4':'vector<float, 4>' '='
    |-DeclRefExpr <col:3> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
    `-BinaryOperator <col:8, col:13> 'vector<float, 4>' '*'
      |-ImplicitCastExpr <col:8> 'vector<float, 4>' <HLSLCC_IntegralToFloating>
      | `-ImplicitCastExpr <col:8> 'int4':'vector<int, 4>' <LValueToRValue>
      |   `-DeclRefExpr <col:8> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
      `-ImplicitCastExpr <col:13> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
        `-ImplicitCastExpr <col:13> 'float' <FloatingCast>
          `-FloatingLiteral <col:13> 'literal float' 5.000000e-01
  */
  m16f = 0.5 * m16f;
  /*verify-ast
    BinaryOperator <col:3, col:16> 'min16float':'half' '='
    |-DeclRefExpr <col:3> 'min16float':'half' lvalue Var 'm16f' 'min16float':'half'
    `-BinaryOperator <col:10, col:16> 'half' '*'
      |-ImplicitCastExpr <col:10> 'half' <FloatingCast>
      | `-FloatingLiteral <col:10> 'literal float' 5.000000e-01
      `-ImplicitCastExpr <col:16> 'min16float':'half' <LValueToRValue>
        `-DeclRefExpr <col:16> 'min16float':'half' lvalue Var 'm16f' 'min16float':'half'
  */
  m16f = 0.5F * m16f;                           /* expected-warning {{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
  /*verify-ast
    BinaryOperator <col:3, col:17> 'min16float':'half' '='
    |-DeclRefExpr <col:3> 'min16float':'half' lvalue Var 'm16f' 'min16float':'half'
    `-ImplicitCastExpr <col:10, col:17> 'min16float':'half' <FloatingCast>
      `-BinaryOperator <col:10, col:17> 'float' '*'
        |-FloatingLiteral <col:10> 'float' 5.000000e-01
        `-ImplicitCastExpr <col:17> 'float' <FloatingCast>
          `-ImplicitCastExpr <col:17> 'min16float':'half' <LValueToRValue>
            `-DeclRefExpr <col:17> 'min16float':'half' lvalue Var 'm16f' 'min16float':'half'
  */
  m16f = 0.5L * m16f;                           /* expected-warning {{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}} fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
  /*verify-ast
    BinaryOperator <col:3, col:17> 'min16float':'half' '='
    |-DeclRefExpr <col:3> 'min16float':'half' lvalue Var 'm16f' 'min16float':'half'
    `-ImplicitCastExpr <col:10, col:17> 'min16float':'half' <FloatingCast>
      `-BinaryOperator <col:10, col:17> 'double' '*'
        |-FloatingLiteral <col:10> 'double' 5.000000e-01
        `-ImplicitCastExpr <col:17> 'double' <FloatingCast>
          `-ImplicitCastExpr <col:17> 'min16float':'half' <LValueToRValue>
            `-DeclRefExpr <col:17> 'min16float':'half' lvalue Var 'm16f' 'min16float':'half'
  */
  m16f4x4 = i4x4 * (m16f + 1);                  /* fxc-warning {{X3205: conversion from larger type to smaller, possible loss of data}} */
  /*verify-ast
    BinaryOperator <col:3, col:29> 'min16float4x4':'matrix<min16float, 4, 4>' '='
    |-DeclRefExpr <col:3> 'min16float4x4':'matrix<min16float, 4, 4>' lvalue Var 'm16f4x4' 'min16float4x4':'matrix<min16float, 4, 4>'
    `-BinaryOperator <col:13, col:29> 'matrix<min16float, 4, 4>' '*'
      |-ImplicitCastExpr <col:13> 'matrix<min16float, 4, 4>' <HLSLCC_IntegralToFloating>
      | `-ImplicitCastExpr <col:13> 'int4x4':'matrix<int, 4, 4>' <LValueToRValue>
      |   `-DeclRefExpr <col:13> 'int4x4':'matrix<int, 4, 4>' lvalue Var 'i4x4' 'int4x4':'matrix<int, 4, 4>'
      `-ImplicitCastExpr <col:20, col:29> 'matrix<min16float, 4, 4>':'matrix<min16float, 4, 4>' <HLSLMatrixSplat>
        `-ParenExpr <col:20, col:29> 'half'
          `-BinaryOperator <col:21, col:28> 'half' '+'
            |-ImplicitCastExpr <col:21> 'min16float':'half' <LValueToRValue>
            | `-DeclRefExpr <col:21> 'min16float':'half' lvalue Var 'm16f' 'min16float':'half'
            `-ImplicitCastExpr <col:28> 'half' <IntegralToFloating>
              `-IntegerLiteral <col:28> 'literal int' 1
  */
  VERIFY_TYPES(min16float4x4, m16f4x4 * (0.5 + 1));
  m16f4x4 = m16f4x4 * (0.5 + 1);
  /*verify-ast
    BinaryOperator <col:3, col:31> 'min16float4x4':'matrix<min16float, 4, 4>' '='
    |-DeclRefExpr <col:3> 'min16float4x4':'matrix<min16float, 4, 4>' lvalue Var 'm16f4x4' 'min16float4x4':'matrix<min16float, 4, 4>'
    `-BinaryOperator <col:13, col:31> 'matrix<min16float, 4, 4>' '*'
      |-ImplicitCastExpr <col:13> 'min16float4x4':'matrix<min16float, 4, 4>' <LValueToRValue>
      | `-DeclRefExpr <col:13> 'min16float4x4':'matrix<min16float, 4, 4>' lvalue Var 'm16f4x4' 'min16float4x4':'matrix<min16float, 4, 4>'
      `-ImplicitCastExpr <col:23, col:31> 'matrix<min16float, 4, 4>':'matrix<min16float, 4, 4>' <HLSLMatrixSplat>
        `-ImplicitCastExpr <col:23, col:31> 'min16float':'half' <FloatingCast>
          `-ParenExpr <col:23, col:31> 'literal float'
            `-BinaryOperator <col:24, col:30> 'literal float' '+'
              |-FloatingLiteral <col:24> 'literal float' 5.000000e-01
              `-ImplicitCastExpr <col:30> 'literal float' <IntegralToFloating>
                `-IntegerLiteral <col:30> 'literal int' 1
  */

  b = i4;                                       /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */
  /*verify-ast
    BinaryOperator <col:3, col:7> 'bool' '='
    |-DeclRefExpr <col:3> 'bool' lvalue Var 'b' 'bool'
    `-ImplicitCastExpr <col:7> 'bool' <IntegralToBoolean>
      `-ImplicitCastExpr <col:7> 'int' <HLSLVectorToScalarCast>
        `-ImplicitCastExpr <col:7> 'vector<int, 1>':'vector<int, 1>' <HLSLVectorTruncationCast>
          `-ImplicitCastExpr <col:7> 'int4':'vector<int, 4>' <LValueToRValue>
            `-DeclRefExpr <col:7> 'int4':'vector<int, 4>' lvalue Var 'i4' 'int4':'vector<int, 4>'
  */

  // TODO: FXC fails this case, but passes other similar cases.  What should we do?
  /*verify-ast
    BinaryOperator <col:3, col:28> 'int' '='
    |-HLSLVectorElementExpr <col:3, col:5> 'int' lvalue vectorcomponent x
    | `-ImplicitCastExpr <col:3> 'vector<int, 1>':'vector<int, 1>' lvalue <HLSLVectorSplat>
    |   `-DeclRefExpr <col:3> 'int' lvalue Var 'i' 'int'
    `-ImplicitCastExpr <col:9, col:28> 'int' <FloatingToIntegral>
      `-ImplicitCastExpr <col:9, col:28> 'float' <HLSLVectorToScalarCast>
        `-ImplicitCastExpr <col:9, col:28> 'vector<float, 1>':'vector<float, 1>' <HLSLVectorTruncationCast>
          `-BinaryOperator <col:9, col:28> 'vector<float, 4>' '+'
            |-ImplicitCastExpr <col:9> 'float4':'vector<float, 4>' <LValueToRValue>
            | `-DeclRefExpr <col:9> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
            `-BinaryOperator <col:14, col:28> 'vector<float, 4>' '/'
              |-BinaryOperator <col:14, col:21> 'vector<float, 4>' '*'
              | |-ImplicitCastExpr <col:14> 'vector<float, 4>':'vector<float, 4>' <HLSLMatrixToVectorCast>
              | | `-ImplicitCastExpr <col:14> 'float1x4':'matrix<float, 1, 4>' <LValueToRValue>
              | |   `-DeclRefExpr <col:14> 'float1x4':'matrix<float, 1, 4>' lvalue Var 'f1x4' 'float1x4':'matrix<float, 1, 4>'
              | `-ImplicitCastExpr <col:21> 'vector<float, 4>':'vector<float, 4>' <HLSLMatrixToVectorCast>
              |   `-ImplicitCastExpr <col:21> 'float4x1':'matrix<float, 4, 1>' <LValueToRValue>
              |     `-DeclRefExpr <col:21> 'float4x1':'matrix<float, 4, 1>' lvalue Var 'f4x1' 'float4x1':'matrix<float, 4, 1>'
              `-ImplicitCastExpr <col:28> 'vector<float, 4>':'vector<float, 4>' <HLSLVectorSplat>
                `-ImplicitCastExpr <col:28> 'float' <IntegralToFloating>
                  `-ImplicitCastExpr <col:28> 'int' <HLSLVectorToScalarCast>
                    `-ImplicitCastExpr <col:28> 'int1':'vector<int, 1>' <LValueToRValue>
                      `-DeclRefExpr <col:28> 'int1':'vector<int, 1>' lvalue Var 'i1' 'int1':'vector<int, 1>'
  */

  // TODO: fxc passes the following (i4x1 should implicitly cast to float4 for mul op)
  f4x4._m02_m11_m20 = i4x1 * f4;                /* expected-warning {{implicit truncation of vector type}} fxc-warning {{X3206: implicit truncation of vector type}} */
  /*verify-ast
    BinaryOperator <col:3, col:30> 'vector<float, 3>':'vector<float, 3>' '='
    |-ExtMatrixElementExpr <col:3, col:8> 'vector<float, 3>':'vector<float, 3>' lvalue vectorcomponent _m02_m11_m20
    | `-DeclRefExpr <col:3> 'float4x4':'matrix<float, 4, 4>' lvalue Var 'f4x4' 'float4x4':'matrix<float, 4, 4>'
    `-ImplicitCastExpr <col:23, col:30> 'vector<float, 3>':'vector<float, 3>' <HLSLMatrixToVectorCast>
      `-ImplicitCastExpr <col:23, col:30> 'matrix<float, 3, 1>':'matrix<float, 3, 1>' <HLSLMatrixTruncationCast>
        `-BinaryOperator <col:23, col:30> 'matrix<float, 4, 1>' '*'
          |-ImplicitCastExpr <col:23> 'matrix<float, 4, 1>' <HLSLCC_IntegralToFloating>
          | `-ImplicitCastExpr <col:23> 'int4x1':'matrix<int, 4, 1>' <LValueToRValue>
          |   `-DeclRefExpr <col:23> 'int4x1':'matrix<int, 4, 1>' lvalue Var 'i4x1' 'int4x1':'matrix<int, 4, 1>'
          `-ImplicitCastExpr <col:30> 'matrix<float, 4, 1>':'matrix<float, 4, 1>' <HLSLVectorToMatrixCast>
            `-ImplicitCastExpr <col:30> 'float4':'vector<float, 4>' <LValueToRValue>
              `-DeclRefExpr <col:30> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
  */


  // TODO: We seem to be missing the vector truncation warning in this case
  f3 = i3x1 * f4;                   /* fxc-warning {{X3206: implicit truncation of vector type}} */
  /*verify-ast
    BinaryOperator <col:3, col:15> 'float3':'vector<float, 3>' '='
    |-DeclRefExpr <col:3> 'float3':'vector<float, 3>' lvalue Var 'f3' 'float3':'vector<float, 3>'
    `-ImplicitCastExpr <col:8, col:15> 'vector<float, 3>':'vector<float, 3>' <HLSLMatrixToVectorCast>
      `-BinaryOperator <col:8, col:15> 'matrix<float, 3, 1>' '*'
        |-ImplicitCastExpr <col:8> 'matrix<float, 3, 1>' <HLSLCC_IntegralToFloating>
        | `-ImplicitCastExpr <col:8> 'int3x1':'matrix<int, 3, 1>' <LValueToRValue>
        |   `-DeclRefExpr <col:8> 'int3x1':'matrix<int, 3, 1>' lvalue Var 'i3x1' 'int3x1':'matrix<int, 3, 1>'
        `-ImplicitCastExpr <col:15> 'matrix<float, 3, 1>':'matrix<float, 3, 1>' <HLSLVectorToMatrixCast>
          `-ImplicitCastExpr <col:15> 'vector<float, 3>':'vector<float, 3>' <HLSLVectorTruncationCast>
            `-ImplicitCastExpr <col:15> 'float4':'vector<float, 4>' <LValueToRValue>
              `-DeclRefExpr <col:15> 'float4':'vector<float, 4>' lvalue Var 'f4' 'float4':'vector<float, 4>'
  */

  // TODO: Fix ternary operator for HLSL vectorized semantics.
  b4 = (b4 * b4) ? b4 : b4;
  /*verify-ast
    BinaryOperator <col:3, col:25> 'bool4':'vector<bool, 4>' '='
    |-DeclRefExpr <col:3> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
    `-ImplicitCastExpr <col:8, col:25> 'bool4':'vector<bool, 4>' <LValueToRValue>
      `-ConditionalOperator <col:8, col:25> 'bool4':'vector<bool, 4>' lvalue
        |-ImplicitCastExpr <col:8, col:16> 'bool' <IntegralToBoolean>
        | `-ImplicitCastExpr <col:8, col:16> 'int' <HLSLVectorToScalarCast>
        |   `-ImplicitCastExpr <col:8, col:16> 'vector<int, 1>':'vector<int, 1>' <HLSLVectorTruncationCast>
        |     `-ParenExpr <col:8, col:16> 'vector<int, 4>'
        |       `-BinaryOperator <col:9, col:14> 'vector<int, 4>' '*'
        |         |-ImplicitCastExpr <col:9> 'vector<int, 4>' <HLSLCC_IntegralCast>
        |         | `-ImplicitCastExpr <col:9> 'bool4':'vector<bool, 4>' <LValueToRValue>
        |         |   `-DeclRefExpr <col:9> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
        |         `-ImplicitCastExpr <col:14> 'bool4':'vector<bool, 4>' <LValueToRValue>
        |           `-DeclRefExpr <col:14> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
        |-DeclRefExpr <col:20> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
        `-DeclRefExpr <col:25> 'bool4':'vector<bool, 4>' lvalue Var 'b4' 'bool4':'vector<bool, 4>'
  */

  return 0.0f;
}

bool1 stresstest() {
  float VarZero = float(1.0f);
  int VarOne = int(2);
  int VarTwo = int(3);
  bool VarThree = bool(4);
  float VarFour = float(5.0f);
  float VarFive = float(6.0f);
  int VarSix = int(7);
  bool3 VarSeven = bool3(8,9,10);
  uint VarEight = uint(11);
  bool4 VarNine = bool4(12,13,14,15);

  return ((bool1)(-(!(-(((((bool4)(VarTwo - VarTwo)) + ((((((bool4)VarThree) * VarNine) * VarNine) ? VarSeven.yyyx : ((bool4)(VarFour * VarZero))) + ((bool4)((((float)VarSix) * VarFour) * (VarZero ? ((float)VarTwo) : ((float)(VarSeven ? ((bool3)VarTwo) : ((bool3)VarEight)).z)))))) * ((bool4)((((bool)VarFour) + (((bool)VarTwo) + VarThree)) ? ((bool)VarSix) : ((bool)((((int)VarNine.w) - VarSix) + ((VarSix + ((int)VarThree)) ? ((int)VarFour) : VarTwo)))))) ? ((bool4)(+(((bool)(~(VarEight ? ((uint)(VarThree ? ((bool)((VarSix - ((int)VarFour)) - VarSix)) : VarSeven.z)) : ((uint)VarNine.z)))) - (!(+(VarOne * ((int)(~(!((((int)VarSeven.y) - VarSix) ? VarTwo : ((int)VarNine.w))))))))))) : ((bool4)(+((VarZero + ((float)VarNine.z)) ? ((float)VarThree) : VarFive))))))).y);
  /*verify-ast
    ReturnStmt <col:3, col:771>
    `-ParenExpr <col:10, col:771> 'bool1':'vector<bool, 1>'
      `-CStyleCastExpr <col:11, col:770> 'bool1':'vector<bool, 1>' <NoOp>
        `-ImplicitCastExpr <col:18, col:770> 'vector<bool, 1>':'vector<bool, 1>' <HLSLVectorSplat>
          `-ImplicitCastExpr <col:18, col:770> 'bool' <IntegralToBoolean>
            `-HLSLVectorElementExpr <col:18, col:770> 'int' y
              `-ParenExpr <col:18, col:768> 'vector<int, 4>'
                `-UnaryOperator <col:19, col:767> 'vector<int, 4>' prefix '-'
                  `-ImplicitCastExpr <col:20, col:767> 'vector<int, 4>' <HLSLCC_IntegralCast>
                    `-ParenExpr <col:20, col:767> 'vector<bool, 4>':'vector<bool, 4>'
                      `-UnaryOperator <col:21, col:766> 'vector<bool, 4>':'vector<bool, 4>' prefix '!'
                        `-ImplicitCastExpr <col:22, col:766> 'vector<bool, 4>' <HLSLCC_IntegralToBoolean>
                          `-ParenExpr <col:22, col:766> 'vector<int, 4>'
                            `-UnaryOperator <col:23, col:765> 'vector<int, 4>' prefix '-'
                              `-ImplicitCastExpr <col:24, col:765> 'vector<int, 4>' <HLSLCC_IntegralCast>
                                `-ParenExpr <col:24, col:765> 'bool4':'vector<bool, 4>'
                                  `-ConditionalOperator <col:25, col:764> 'bool4':'vector<bool, 4>'
                                    |-ImplicitCastExpr <col:25, col:457> 'bool' <IntegralToBoolean>
                                    | `-ImplicitCastExpr <col:25, col:457> 'int' <HLSLVectorToScalarCast>
                                    |   `-ImplicitCastExpr <col:25, col:457> 'vector<int, 1>':'vector<int, 1>' <HLSLVectorTruncationCast>
                                    |     `-ParenExpr <col:25, col:457> 'vector<int, 4>'
                                    |       `-BinaryOperator <col:26, col:456> 'vector<int, 4>' '*'
                                    |         |-ParenExpr <col:26, col:281> 'vector<int, 4>'
                                    |         | `-BinaryOperator <col:27, col:280> 'vector<int, 4>' '+'
                                    |         |   |-ImplicitCastExpr <col:27, col:52> 'vector<int, 4>' <HLSLCC_IntegralCast>
                                    |         |   | `-ParenExpr <col:27, col:52> 'bool4':'vector<bool, 4>'
                                    |         |   |   `-CStyleCastExpr <col:28, col:51> 'bool4':'vector<bool, 4>' <NoOp>
                                    |         |   |     `-ImplicitCastExpr <col:35, col:51> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
                                    |         |   |       `-ImplicitCastExpr <col:35, col:51> 'bool' <IntegralToBoolean>
                                    |         |   |         `-ParenExpr <col:35, col:51> 'int'
                                    |         |   |           `-BinaryOperator <col:36, col:45> 'int' '-'
                                    |         |   |             |-ImplicitCastExpr <col:36> 'int' <LValueToRValue>
                                    |         |   |             | `-DeclRefExpr <col:36> 'int' lvalue Var 'VarTwo' 'int'
                                    |         |   |             `-ImplicitCastExpr <col:45> 'int' <LValueToRValue>
                                    |         |   |               `-DeclRefExpr <col:45> 'int' lvalue Var 'VarTwo' 'int'
                                    |         |   `-ParenExpr <col:56, col:280> 'vector<int, 4>'
                                    |         |     `-BinaryOperator <col:57, col:279> 'vector<int, 4>' '+'
                                    |         |       |-ImplicitCastExpr <col:57, col:146> 'vector<int, 4>' <HLSLCC_IntegralCast>
                                    |         |       | `-ParenExpr <col:57, col:146> 'vector<bool, 4>':'vector<bool, 4>'
                                    |         |       |   `-ConditionalOperator <col:58, col:145> 'vector<bool, 4>':'vector<bool, 4>'
                                    |         |       |     |-ImplicitCastExpr <col:58, col:98> 'bool' <IntegralToBoolean>
                                    |         |       |     | `-ImplicitCastExpr <col:58, col:98> 'int' <HLSLVectorToScalarCast>
                                    |         |       |     |   `-ImplicitCastExpr <col:58, col:98> 'vector<int, 1>':'vector<int, 1>' <HLSLVectorTruncationCast>
                                    |         |       |     |     `-ParenExpr <col:58, col:98> 'vector<int, 4>'
                                    |         |       |     |       `-BinaryOperator <col:59, col:91> 'vector<int, 4>' '*'
                                    |         |       |     |         |-ParenExpr <col:59, col:87> 'vector<int, 4>'
                                    |         |       |     |         | `-BinaryOperator <col:60, col:80> 'vector<int, 4>' '*'
                                    |         |       |     |         |   |-ImplicitCastExpr <col:60, col:76> 'vector<int, 4>' <HLSLCC_IntegralCast>
                                    |         |       |     |         |   | `-ParenExpr <col:60, col:76> 'bool4':'vector<bool, 4>'
                                    |         |       |     |         |   |   `-CStyleCastExpr <col:61, col:68> 'bool4':'vector<bool, 4>' <NoOp>
                                    |         |       |     |         |   |     `-ImplicitCastExpr <col:68> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
                                    |         |       |     |         |   |       `-ImplicitCastExpr <col:68> 'bool' <LValueToRValue>
                                    |         |       |     |         |   |         `-DeclRefExpr <col:68> 'bool' lvalue Var 'VarThree' 'bool'
                                    |         |       |     |         |   `-ImplicitCastExpr <col:80> 'bool4':'vector<bool, 4>' <LValueToRValue>
                                    |         |       |     |         |     `-DeclRefExpr <col:80> 'bool4':'vector<bool, 4>' lvalue Var 'VarNine' 'bool4':'vector<bool, 4>'
                                    |         |       |     |         `-ImplicitCastExpr <col:91> 'bool4':'vector<bool, 4>' <LValueToRValue>
                                    |         |       |     |           `-DeclRefExpr <col:91> 'bool4':'vector<bool, 4>' lvalue Var 'VarNine' 'bool4':'vector<bool, 4>'
                                    |         |       |     |-HLSLVectorElementExpr <col:102, col:111> 'vector<bool, 4>':'vector<bool, 4>' yyyx
                                    |         |       |     | `-DeclRefExpr <col:102> 'bool3':'vector<bool, 3>' lvalue Var 'VarSeven' 'bool3':'vector<bool, 3>'
                                    |         |       |     `-ParenExpr <col:118, col:145> 'bool4':'vector<bool, 4>'
                                    |         |       |       `-CStyleCastExpr <col:119, col:144> 'bool4':'vector<bool, 4>' <NoOp>
                                    |         |       |         `-ImplicitCastExpr <col:126, col:144> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
                                    |         |       |           `-ImplicitCastExpr <col:126, col:144> 'bool' <FloatingToBoolean>
                                    |         |       |             `-ParenExpr <col:126, col:144> 'float'
                                    |         |       |               `-BinaryOperator <col:127, col:137> 'float' '*'
                                    |         |       |                 |-ImplicitCastExpr <col:127> 'float' <LValueToRValue>
                                    |         |       |                 | `-DeclRefExpr <col:127> 'float' lvalue Var 'VarFour' 'float'
                                    |         |       |                 `-ImplicitCastExpr <col:137> 'float' <LValueToRValue>
                                    |         |       |                   `-DeclRefExpr <col:137> 'float' lvalue Var 'VarZero' 'float'
                                    |         |       `-ParenExpr <col:150, col:279> 'bool4':'vector<bool, 4>'
                                    |         |         `-CStyleCastExpr <col:151, col:278> 'bool4':'vector<bool, 4>' <NoOp>
                                    |         |           `-ImplicitCastExpr <col:158, col:278> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
                                    |         |             `-ImplicitCastExpr <col:158, col:278> 'bool' <FloatingToBoolean>
                                    |         |               `-ParenExpr <col:158, col:278> 'float'
                                    |         |                 `-BinaryOperator <col:159, col:277> 'float' '*'
                                    |         |                   |-ParenExpr <col:159, col:185> 'float'
                                    |         |                   | `-BinaryOperator <col:160, col:178> 'float' '*'
                                    |         |                   |   |-ParenExpr <col:160, col:174> 'float'
                                    |         |                   |   | `-CStyleCastExpr <col:161, col:168> 'float' <NoOp>
                                    |         |                   |   |   `-ImplicitCastExpr <col:168> 'float' <IntegralToFloating>
                                    |         |                   |   |     `-ImplicitCastExpr <col:168> 'int' <LValueToRValue>
                                    |         |                   |   |       `-DeclRefExpr <col:168> 'int' lvalue Var 'VarSix' 'int'
                                    |         |                   |   `-ImplicitCastExpr <col:178> 'float' <LValueToRValue>
                                    |         |                   |     `-DeclRefExpr <col:178> 'float' lvalue Var 'VarFour' 'float'
                                    |         |                   `-ParenExpr <col:189, col:277> 'float'
                                    |         |                     `-ConditionalOperator <col:190, col:276> 'float'
                                    |         |                       |-ImplicitCastExpr <col:190> 'bool' <FloatingToBoolean>
                                    |         |                       | `-ImplicitCastExpr <col:190> 'float' <LValueToRValue>
                                    |         |                       |   `-DeclRefExpr <col:190> 'float' lvalue Var 'VarZero' 'float'
                                    |         |                       |-ParenExpr <col:200, col:214> 'float'
                                    |         |                       | `-CStyleCastExpr <col:201, col:208> 'float' <NoOp>
                                    |         |                       |   `-ImplicitCastExpr <col:208> 'float' <IntegralToFloating>
                                    |         |                       |     `-ImplicitCastExpr <col:208> 'int' <LValueToRValue>
                                    |         |                       |       `-DeclRefExpr <col:208> 'int' lvalue Var 'VarTwo' 'int'
                                    |         |                       `-ParenExpr <col:218, col:276> 'float'
                                    |         |                         `-CStyleCastExpr <col:219, col:275> 'float' <NoOp>
                                    |         |                           `-HLSLVectorElementExpr <col:226, col:275> 'bool' z
                                    |         |                             `-ParenExpr <col:226, col:273> 'bool3':'vector<bool, 3>'
                                    |         |                               `-ConditionalOperator <col:227, col:272> 'bool3':'vector<bool, 3>'
                                    |         |                                 |-ImplicitCastExpr <col:227> 'bool' lvalue <HLSLVectorToScalarCast>
                                    |         |                                 | `-ImplicitCastExpr <col:227> 'vector<bool, 1>':'vector<bool, 1>' lvalue <HLSLVectorTruncationCast>
                                    |         |                                 |   `-DeclRefExpr <col:227> 'bool3':'vector<bool, 3>' lvalue Var 'VarSeven' 'bool3':'vector<bool, 3>'
                                    |         |                                 |-ParenExpr <col:238, col:252> 'bool3':'vector<bool, 3>'
                                    |         |                                 | `-CStyleCastExpr <col:239, col:246> 'bool3':'vector<bool, 3>' <NoOp>
                                    |         |                                 |   `-ImplicitCastExpr <col:246> 'vector<bool, 3>':'vector<bool, 3>' <HLSLVectorSplat>
                                    |         |                                 |     `-ImplicitCastExpr <col:246> 'bool' <IntegralToBoolean>
                                    |         |                                 |       `-ImplicitCastExpr <col:246> 'int' <LValueToRValue>
                                    |         |                                 |         `-DeclRefExpr <col:246> 'int' lvalue Var 'VarTwo' 'int'
                                    |         |                                 `-ParenExpr <col:256, col:272> 'bool3':'vector<bool, 3>'
                                    |         |                                   `-CStyleCastExpr <col:257, col:264> 'bool3':'vector<bool, 3>' <NoOp>
                                    |         |                                     `-ImplicitCastExpr <col:264> 'vector<bool, 3>':'vector<bool, 3>' <HLSLVectorSplat>
                                    |         |                                       `-ImplicitCastExpr <col:264> 'bool' <IntegralToBoolean>
                                    |         |                                         `-ImplicitCastExpr <col:264> 'uint':'unsigned int' <LValueToRValue>
                                    |         |                                           `-DeclRefExpr <col:264> 'uint':'unsigned int' lvalue Var 'VarEight' 'uint':'unsigned int'
                                    |         `-ParenExpr <col:285, col:456> 'bool4':'vector<bool, 4>'
                                    |           `-CStyleCastExpr <col:286, col:455> 'bool4':'vector<bool, 4>' <NoOp>
                                    |             `-ImplicitCastExpr <col:293, col:455> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
                                    |               `-ParenExpr <col:293, col:455> 'bool'
                                    |                 `-ConditionalOperator <col:294, col:454> 'bool'
                                    |                   |-ImplicitCastExpr <col:294, col:340> 'bool' <IntegralToBoolean>
                                    |                   | `-ParenExpr <col:294, col:340> 'int'
                                    |                   |   `-BinaryOperator <col:295, col:339> 'int' '+'
                                    |                   |     |-ImplicitCastExpr <col:295, col:309> 'int' <IntegralCast>
                                    |                   |     | `-ParenExpr <col:295, col:309> 'bool'
                                    |                   |     |   `-CStyleCastExpr <col:296, col:302> 'bool' <NoOp>
                                    |                   |     |     `-ImplicitCastExpr <col:302> 'bool' <FloatingToBoolean>
                                    |                   |     |       `-ImplicitCastExpr <col:302> 'float' <LValueToRValue>
                                    |                   |     |         `-DeclRefExpr <col:302> 'float' lvalue Var 'VarFour' 'float'
                                    |                   |     `-ParenExpr <col:313, col:339> 'int'
                                    |                   |       `-BinaryOperator <col:314, col:331> 'int' '+'
                                    |                   |         |-ImplicitCastExpr <col:314, col:327> 'int' <IntegralCast>
                                    |                   |         | `-ParenExpr <col:314, col:327> 'bool'
                                    |                   |         |   `-CStyleCastExpr <col:315, col:321> 'bool' <NoOp>
                                    |                   |         |     `-ImplicitCastExpr <col:321> 'bool' <IntegralToBoolean>
                                    |                   |         |       `-ImplicitCastExpr <col:321> 'int' <LValueToRValue>
                                    |                   |         |         `-DeclRefExpr <col:321> 'int' lvalue Var 'VarTwo' 'int'
                                    |                   |         `-ImplicitCastExpr <col:331> 'bool' <LValueToRValue>
                                    |                   |           `-DeclRefExpr <col:331> 'bool' lvalue Var 'VarThree' 'bool'
                                    |                   |-ParenExpr <col:344, col:357> 'bool'
                                    |                   | `-CStyleCastExpr <col:345, col:351> 'bool' <NoOp>
                                    |                   |   `-ImplicitCastExpr <col:351> 'bool' <IntegralToBoolean>
                                    |                   |     `-ImplicitCastExpr <col:351> 'int' <LValueToRValue>
                                    |                   |       `-DeclRefExpr <col:351> 'int' lvalue Var 'VarSix' 'int'
                                    |                   `-ParenExpr <col:361, col:454> 'bool'
                                    |                     `-CStyleCastExpr <col:362, col:453> 'bool' <NoOp>
                                    |                       `-ImplicitCastExpr <col:368, col:453> 'bool' <IntegralToBoolean>
                                    |                         `-ParenExpr <col:368, col:453> 'int'
                                    |                           `-BinaryOperator <col:369, col:452> 'int' '+'
                                    |                             |-ParenExpr <col:369, col:395> 'int'
                                    |                             | `-BinaryOperator <col:370, col:389> 'int' '-'
                                    |                             |   |-ParenExpr <col:370, col:385> 'int'
                                    |                             |   | `-CStyleCastExpr <col:371, col:384> 'int' <NoOp>
                                    |                             |   |   `-ImplicitCastExpr <col:376, col:384> 'bool' <LValueToRValue>
                                    |                             |   |     `-HLSLVectorElementExpr <col:376, col:384> 'bool' lvalue vectorcomponent w
                                    |                             |   |       `-DeclRefExpr <col:376> 'bool4':'vector<bool, 4>' lvalue Var 'VarNine' 'bool4':'vector<bool, 4>'
                                    |                             |   `-ImplicitCastExpr <col:389> 'int' <LValueToRValue>
                                    |                             |     `-DeclRefExpr <col:389> 'int' lvalue Var 'VarSix' 'int'
                                    |                             `-ParenExpr <col:399, col:452> 'int'
                                    |                               `-ConditionalOperator <col:400, col:446> 'int'
                                    |                                 |-ImplicitCastExpr <col:400, col:425> 'bool' <IntegralToBoolean>
                                    |                                 | `-ParenExpr <col:400, col:425> 'int'
                                    |                                 |   `-BinaryOperator <col:401, col:424> 'int' '+'
                                    |                                 |     |-ImplicitCastExpr <col:401> 'int' <LValueToRValue>
                                    |                                 |     | `-DeclRefExpr <col:401> 'int' lvalue Var 'VarSix' 'int'
                                    |                                 |     `-ParenExpr <col:410, col:424> 'int'
                                    |                                 |       `-CStyleCastExpr <col:411, col:416> 'int' <NoOp>
                                    |                                 |         `-ImplicitCastExpr <col:416> 'bool' <LValueToRValue>
                                    |                                 |           `-DeclRefExpr <col:416> 'bool' lvalue Var 'VarThree' 'bool'
                                    |                                 |-ParenExpr <col:429, col:442> 'int'
                                    |                                 | `-CStyleCastExpr <col:430, col:435> 'int' <NoOp>
                                    |                                 |   `-ImplicitCastExpr <col:435> 'int' <FloatingToIntegral>
                                    |                                 |     `-ImplicitCastExpr <col:435> 'float' <LValueToRValue>
                                    |                                 |       `-DeclRefExpr <col:435> 'float' lvalue Var 'VarFour' 'float'
                                    |                                 `-ImplicitCastExpr <col:446> 'int' <LValueToRValue>
                                    |                                   `-DeclRefExpr <col:446> 'int' lvalue Var 'VarTwo' 'int'
                                    |-ParenExpr <col:461, col:687> 'bool4':'vector<bool, 4>'
                                    | `-CStyleCastExpr <col:462, col:686> 'bool4':'vector<bool, 4>' <NoOp>
                                    |   `-ImplicitCastExpr <col:469, col:686> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
                                    |     `-ImplicitCastExpr <col:469, col:686> 'bool' <IntegralToBoolean>
                                    |       `-ParenExpr <col:469, col:686> 'int'
                                    |         `-UnaryOperator <col:470, col:685> 'int' prefix '+'
                                    |           `-ParenExpr <col:471, col:685> 'int'
                                    |             `-BinaryOperator <col:472, col:684> 'int' '-'
                                    |               |-ImplicitCastExpr <col:472, col:593> 'int' <IntegralCast>
                                    |               | `-ParenExpr <col:472, col:593> 'bool'
                                    |               |   `-CStyleCastExpr <col:473, col:592> 'bool' <NoOp>
                                    |               |     `-ImplicitCastExpr <col:479, col:592> 'bool' <IntegralToBoolean>
                                    |               |       `-ParenExpr <col:479, col:592> 'uint':'unsigned int'
                                    |               |         `-UnaryOperator <col:480, col:591> 'uint':'unsigned int' prefix '~'
                                    |               |           `-ParenExpr <col:481, col:591> 'uint':'unsigned int'
                                    |               |             `-ConditionalOperator <col:482, col:590> 'uint':'unsigned int'
                                    |               |               |-ImplicitCastExpr <col:482> 'bool' <IntegralToBoolean>
                                    |               |               | `-ImplicitCastExpr <col:482> 'uint':'unsigned int' <LValueToRValue>
                                    |               |               |   `-DeclRefExpr <col:482> 'uint':'unsigned int' lvalue Var 'VarEight' 'uint':'unsigned int'
                                    |               |               |-ParenExpr <col:493, col:570> 'uint':'unsigned int'
                                    |               |               | `-CStyleCastExpr <col:494, col:569> 'uint':'unsigned int' <NoOp>
                                    |               |               |   `-ParenExpr <col:500, col:569> 'bool'
                                    |               |               |     `-ConditionalOperator <col:501, col:568> 'bool'
                                    |               |               |       |-ImplicitCastExpr <col:501> 'bool' <LValueToRValue>
                                    |               |               |       | `-DeclRefExpr <col:501> 'bool' lvalue Var 'VarThree' 'bool'
                                    |               |               |       |-ParenExpr <col:512, col:555> 'bool'
                                    |               |               |       | `-CStyleCastExpr <col:513, col:554> 'bool' <NoOp>
                                    |               |               |       |   `-ImplicitCastExpr <col:519, col:554> 'bool' <IntegralToBoolean>
                                    |               |               |       |     `-ParenExpr <col:519, col:554> 'int'
                                    |               |               |       |       `-BinaryOperator <col:520, col:548> 'int' '-'
                                    |               |               |       |         |-ParenExpr <col:520, col:544> 'int'
                                    |               |               |       |         | `-BinaryOperator <col:521, col:543> 'int' '-'
                                    |               |               |       |         |   |-ImplicitCastExpr <col:521> 'int' <LValueToRValue>
                                    |               |               |       |         |   | `-DeclRefExpr <col:521> 'int' lvalue Var 'VarSix' 'int'
                                    |               |               |       |         |   `-ParenExpr <col:530, col:543> 'int'
                                    |               |               |       |         |     `-CStyleCastExpr <col:531, col:536> 'int' <NoOp>
                                    |               |               |       |         |       `-ImplicitCastExpr <col:536> 'int' <FloatingToIntegral>
                                    |               |               |       |         |         `-ImplicitCastExpr <col:536> 'float' <LValueToRValue>
                                    |               |               |       |         |           `-DeclRefExpr <col:536> 'float' lvalue Var 'VarFour' 'float'
                                    |               |               |       |         `-ImplicitCastExpr <col:548> 'int' <LValueToRValue>
                                    |               |               |       |           `-DeclRefExpr <col:548> 'int' lvalue Var 'VarSix' 'int'
                                    |               |               |       `-ImplicitCastExpr <col:559, col:568> 'bool' <LValueToRValue>
                                    |               |               |         `-HLSLVectorElementExpr <col:559, col:568> 'bool' lvalue vectorcomponent z
                                    |               |               |           `-DeclRefExpr <col:559> 'bool3':'vector<bool, 3>' lvalue Var 'VarSeven' 'bool3':'vector<bool, 3>'
                                    |               |               `-ParenExpr <col:574, col:590> 'uint':'unsigned int'
                                    |               |                 `-CStyleCastExpr <col:575, col:589> 'uint':'unsigned int' <NoOp>
                                    |               |                   `-ImplicitCastExpr <col:581, col:589> 'bool' <LValueToRValue>
                                    |               |                     `-HLSLVectorElementExpr <col:581, col:589> 'bool' lvalue vectorcomponent z
                                    |               |                       `-DeclRefExpr <col:581> 'bool4':'vector<bool, 4>' lvalue Var 'VarNine' 'bool4':'vector<bool, 4>'
                                    |               `-ParenExpr <col:597, col:684> 'bool'
                                    |                 `-UnaryOperator <col:598, col:683> 'bool' prefix '!'
                                    |                   `-ImplicitCastExpr <col:599, col:683> 'bool' <IntegralToBoolean>
                                    |                     `-ParenExpr <col:599, col:683> 'int'
                                    |                       `-UnaryOperator <col:600, col:682> 'int' prefix '+'
                                    |                         `-ParenExpr <col:601, col:682> 'int'
                                    |                           `-BinaryOperator <col:602, col:681> 'int' '*'
                                    |                             |-ImplicitCastExpr <col:602> 'int' <LValueToRValue>
                                    |                             | `-DeclRefExpr <col:602> 'int' lvalue Var 'VarOne' 'int'
                                    |                             `-ParenExpr <col:611, col:681> 'int'
                                    |                               `-CStyleCastExpr <col:612, col:680> 'int' <NoOp>
                                    |                                 `-ParenExpr <col:617, col:680> 'int'
                                    |                                   `-UnaryOperator <col:618, col:679> 'int' prefix '~'
                                    |                                     `-ImplicitCastExpr <col:619, col:679> 'int' <IntegralCast>
                                    |                                       `-ParenExpr <col:619, col:679> 'bool'
                                    |                                         `-UnaryOperator <col:620, col:678> 'bool' prefix '!'
                                    |                                           `-ImplicitCastExpr <col:621, col:678> 'bool' <IntegralToBoolean>
                                    |                                             `-ParenExpr <col:621, col:678> 'int'
                                    |                                               `-ConditionalOperator <col:622, col:677> 'int'
                                    |                                                 |-ImplicitCastExpr <col:622, col:649> 'bool' <IntegralToBoolean>
                                    |                                                 | `-ParenExpr <col:622, col:649> 'int'
                                    |                                                 |   `-BinaryOperator <col:623, col:643> 'int' '-'
                                    |                                                 |     |-ParenExpr <col:623, col:639> 'int'
                                    |                                                 |     | `-CStyleCastExpr <col:624, col:638> 'int' <NoOp>
                                    |                                                 |     |   `-ImplicitCastExpr <col:629, col:638> 'bool' <LValueToRValue>
                                    |                                                 |     |     `-HLSLVectorElementExpr <col:629, col:638> 'bool' lvalue vectorcomponent y
                                    |                                                 |     |       `-DeclRefExpr <col:629> 'bool3':'vector<bool, 3>' lvalue Var 'VarSeven' 'bool3':'vector<bool, 3>'
                                    |                                                 |     `-ImplicitCastExpr <col:643> 'int' <LValueToRValue>
                                    |                                                 |       `-DeclRefExpr <col:643> 'int' lvalue Var 'VarSix' 'int'
                                    |                                                 |-ImplicitCastExpr <col:653> 'int' <LValueToRValue>
                                    |                                                 | `-DeclRefExpr <col:653> 'int' lvalue Var 'VarTwo' 'int'
                                    |                                                 `-ParenExpr <col:662, col:677> 'int'
                                    |                                                   `-CStyleCastExpr <col:663, col:676> 'int' <NoOp>
                                    |                                                     `-ImplicitCastExpr <col:668, col:676> 'bool' <LValueToRValue>
                                    |                                                       `-HLSLVectorElementExpr <col:668, col:676> 'bool' lvalue vectorcomponent w
                                    |                                                         `-DeclRefExpr <col:668> 'bool4':'vector<bool, 4>' lvalue Var 'VarNine' 'bool4':'vector<bool, 4>'
                                    `-ParenExpr <col:691, col:764> 'bool4':'vector<bool, 4>'
                                      `-CStyleCastExpr <col:692, col:763> 'bool4':'vector<bool, 4>' <NoOp>
                                        `-ImplicitCastExpr <col:699, col:763> 'vector<bool, 4>':'vector<bool, 4>' <HLSLVectorSplat>
                                          `-ImplicitCastExpr <col:699, col:763> 'bool' <FloatingToBoolean>
                                            `-ParenExpr <col:699, col:763> 'float'
                                              `-UnaryOperator <col:700, col:762> 'float' prefix '+'
                                                `-ParenExpr <col:701, col:762> 'float'
                                                  `-ConditionalOperator <col:702, col:755> 'float'
                                                    |-ImplicitCastExpr <col:702, col:731> 'bool' <FloatingToBoolean>
                                                    | `-ParenExpr <col:702, col:731> 'float'
                                                    |   `-BinaryOperator <col:703, col:730> 'float' '+'
                                                    |     |-ImplicitCastExpr <col:703> 'float' <LValueToRValue>
                                                    |     | `-DeclRefExpr <col:703> 'float' lvalue Var 'VarZero' 'float'
                                                    |     `-ParenExpr <col:713, col:730> 'float'
                                                    |       `-CStyleCastExpr <col:714, col:729> 'float' <NoOp>
                                                    |         `-ImplicitCastExpr <col:721, col:729> 'bool' <LValueToRValue>
                                                    |           `-HLSLVectorElementExpr <col:721, col:729> 'bool' lvalue vectorcomponent z
                                                    |             `-DeclRefExpr <col:721> 'bool4':'vector<bool, 4>' lvalue Var 'VarNine' 'bool4':'vector<bool, 4>'
                                                    |-ParenExpr <col:735, col:751> 'float'
                                                    | `-CStyleCastExpr <col:736, col:743> 'float' <NoOp>
                                                    |   `-ImplicitCastExpr <col:743> 'bool' <LValueToRValue>
                                                    |     `-DeclRefExpr <col:743> 'bool' lvalue Var 'VarThree' 'bool'
                                                    `-ImplicitCastExpr <col:755> 'float' <LValueToRValue>
                                                      `-DeclRefExpr <col:755> 'float' lvalue Var 'VarFive' 'float'
  */
}

float4 main() : SV_Target
{
  return test();
}
