// RUN: %dxc -T vs_6_0 -ast-dump %s | FileCheck %s
// RUN: %dxc -T vs_6_0 %s | FileCheck -check-prefix=CHECKIR %s

// sizeof() results in unsigned long, while various HLSL intrinsics take unsigned int.
// These types are identical in HLSL (uint), but not in clang.
// A conversion step is still necessary to prevent this case from causing
// assert when checking function arguments during codegen.

// CHECK: CallExpr
// CHECK-NEXT: ImplicitCastExpr
// CHECK-NEXT: DeclRefExpr
// CHECK-NEXT: ImplicitCastExpr
// CHECK-SAME: IntegralCast
// CHECK-NEXT: UnaryExprOrTypeTraitExpr

// CHECKIR: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 4)

uint main() : OUT {
  return abs(sizeof(float));
}
