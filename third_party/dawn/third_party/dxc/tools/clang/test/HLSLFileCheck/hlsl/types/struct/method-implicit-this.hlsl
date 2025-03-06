// RUN: %dxc -T ps_6_0 -ast-dump %s | FileCheck %s

struct Foo {
  float m;
  float2 f(float2 v) { return 0; }
  float3 f(float3 v) { return 1; }
  float2 g(float2 v) { return f(v); }
};

// CHECK:     CXXMemberCallExpr 0x{{[0-9a-zA-Z]+}} <col:31, col:34> 'float2':'vector<float, 2>'
// CHECK-NEXT: MemberExpr 0x{{[0-9a-zA-Z]+}} <col:31> '<bound member function type>' .f
// CHECK-NEXT: CXXThisExpr 0x{{[0-9a-zA-Z]+}} <col:31> 'Foo' lvalue this
// CHECK-NEXT: ImplicitCastExpr 0x{{[0-9a-zA-Z]+}} <col:33> 'float2':'vector<float, 2>' <LValueToRValue>
// CHECK-NEXT: DeclRefExpr 0x{{[0-9a-zA-Z]+}} <col:33> 'float2':'vector<float, 2>' lvalue ParmVar 0x{{[0-9a-zA-Z]+}} 'v' 'float2':'vector<float, 2>'

float4 main(float2 coord: TEXCOORD) : SV_TARGET {
  Foo foo = { coord.x };
  return float4(foo.g(coord.y), 0, 1);
}
