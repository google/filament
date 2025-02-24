// RUN: %dxc -E main -T ps_6_0 %s  -ast-dump | FileCheck %s

// Check matrix attribute.

struct Mat
{
// CHECK:-FieldDecl {{.*}}, col:22> col:22 referenced m 'row_major float2x2':'matrix<float, 2, 2>'
// CHECK-NOT: |-HLSLRowMajorAttr 0x{{.*}} <col:3>
  row_major float2x2 m;
};

Mat m;
// CHECK:|-VarDecl {{.*}}, col:10> col:10 used t 'const float3x2':'const matrix<float, 3, 2>'
float3x2 t;

// CHECK:|-VarDecl {{.*}}, col:10> col:10 used tt 'const float3x3':'const matrix<float, 3, 3>'
float3x3 tt;

// CHECK:|-FunctionDecl 0x{{.*}}:20 used foo 'row_major float3x3 ()'
// CHECK-NEXT:CompoundStmt 0x{{.*}} <col:26,
// CHECK-NEXT:| `-ReturnStmt 0x{{.*}}, col:10>
// CHECK-NEXT:|   `-ImplicitCastExpr 0x{{.*}} <col:10> 'float3x3':'matrix<float, 3, 3>' <LValueToRValue>
// CHECK-NEXT:|     `-DeclRefExpr 0x{{.*}} <col:10> 'const float3x3':'const matrix<float, 3, 3>' lvalue Var 0x{{.*}} 'tt' 'const float3x3':'const matrix<float, 3, 3>'
// CHECK-NOT:|-HLSLRowMajorAttr
row_major float3x3 foo() {
  return tt;
}

// CHECK:-FunctionDecl 0x{{.*}} main 'float4 (column_major float4x4)'
// CHECK-NEXT:|-ParmVarDecl 0x{{.*}} <col:13, col:35> col:35 used m2 'column_major float4x4':'matrix<float, 4, 4>'
// CHECK-NOT:|-HLSLColumnMajorAttr
float4 main(column_major float4x4 m2 : M) :SV_Target {
    Mat lm = m;
    return lm.m + t[0].xxxx + m2[1] + foo()[1].xxxx;
}
