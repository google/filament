// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -fcgl %s | FileCheck %s
// RUN: %dxc -T lib_6_10 %s | FileCheck %s --check-prefix=CHECKVAL

#include <dx/linalg.h>
using namespace dx::linalg;

// CHECK: %"class.dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::I32, 4, 5, dx::linalg::MatrixUse::MatrixUseEnum::B,
// CHECK-SAME: dx::linalg::MatrixScope::MatrixScopeEnum::ThreadGroup>" = type { %dx.types.LinAlgMatrixC4M4N5U1S2 }
// CHECK: %dx.types.LinAlgMatrixC4M4N5U1S2 = type { i8* }

// CHECK: %"class.dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::F32, 100, 100, dx::linalg::MatrixUse::MatrixUseEnum::A,
// CHECK-SAME: dx::linalg::MatrixScope::MatrixScopeEnum::Wave>" = type { %dx.types.LinAlgMatrixC9M100N100U0S1 }
// CHECK: %dx.types.LinAlgMatrixC9M100N100U0S1 = type { i8* }

// CHECK: define internal void @"\01?f@@YAXXZ"()
void f() {
// CHECK:  %mat1 = alloca %"class.dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::I32, 4, 5,
// CHECK-SAME: dx::linalg::MatrixUse::MatrixUseEnum::B, dx::linalg::MatrixScope::MatrixScopeEnum::ThreadGroup>", align 4
  Matrix<ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup> mat1;
// CHECK:  %mat2 = alloca %"class.dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::F32, 100, 100,
// CHECK-SAME: dx::linalg::MatrixUse::MatrixUseEnum::A, dx::linalg::MatrixScope::MatrixScopeEnum::Wave>", align 4
  Matrix<ComponentType::F32, 100, 100, MatrixUse::A, MatrixScope::Wave> mat2;
}

// CHECK: !dx.targetTypes = !{![[T0:.*]], ![[T0:.*]]}
// CHECK: ![[T0:.*]] = !{%dx.types.LinAlgMatrixC4M4N5U1S2 undef, i32 4, i32 4, i32 5, i32 1, i32 2}
// CHECK: ![[T1:.*]] = !{%dx.types.LinAlgMatrixC9M100N100U0S1 undef, i32 9, i32 100, i32 100, i32 0, i32 1}

// CHECKVAL-NOT: error: validation errors
// CHECKVAL-NOT: error: Named metadata 'dx.targetTypes' is unknown.
