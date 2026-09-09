// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -enable-16bit-types -fcgl %s | FileCheck %s
// RUN: %dxc -T lib_6_10 -enable-16bit-types %s | FileCheck %s --check-prefix=CHECKVAL

#include <dx/linalg.h>
using namespace dx::linalg;

// CHECK: %dx.types.LinAlgMatrixC4M4N5U1S2 = type { i8* }
// CHECK: %dx.types.LinAlgMatrixC21M3N3U0S1 = type { i8* }
// CHECK: %dx.types.LinAlgMatrixC9M10N20U0S0 = type { i8* }
// CHECK: %dx.types.LinAlgMatrixC2M3N4U2S2 = type { i8* }

// CHECK: define internal void @"\01?f1@@YAXXZ"()
// CHECK: %{{.*}} = alloca %dx.types.LinAlgMatrixC4M4N5U1S2, align 4
void f1() {
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]] mat1;
}

// CHECK: define internal void @"\01?f2@@YAX$linalg_matrixC21M3N3U0S1@@Z"(%dx.types.LinAlgMatrixC21M3N3U0S1 %mat2.coerce)
// CHECK: %{{.*}} = alloca %dx.types.LinAlgMatrixC21M3N3U0S1, align 4
void f2(__builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::F8_E4M3FN, 3, 3, MatrixUse::A, MatrixScope::Wave)]] mat2) {
}

// CHECK: define internal %dx.types.LinAlgMatrixC9M10N20U0S0 @"\01?f3@@YA$linalg_matrixC9M10N20U0S0@XZ"()
// CHECK: %{{.*}} = alloca %dx.types.LinAlgMatrixC9M10N20U0S0, align 4
typedef __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Thread)]] Mat10by20;

Mat10by20 f3() {
  Mat10by20 mat3;
  return mat3;
}

// CHECK: define internal void @"\01?f4@@YAXXZ"()
// CHECK: call void @"\01??$fTemplate@$02$03@@YAXXZ"()

// CHECK: define linkonce_odr void @"\01??$fTemplate@$02$03@@YAXXZ"()
// CHECK: %mat4 = alloca %dx.types.LinAlgMatrixC2M3N4U2S2, align 4

template <uint M, uint N>
void fTemplate() {
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I16, M, N, MatrixUse::Accumulator, MatrixScope::ThreadGroup)]] mat4;
}

void f4() {
  fTemplate<3, 4>();
}

// CHECK: !dx.targetTypes = !{![[T0:.*]], ![[T1:.*]], ![[T2:.*]], ![[T3:.*]]}
// CHECK: ![[T0:.*]] = !{%dx.types.LinAlgMatrixC4M4N5U1S2 undef, i32 4, i32 4, i32 5, i32 1, i32 2}
// CHECK: ![[T1:.*]] = !{%dx.types.LinAlgMatrixC21M3N3U0S1 undef, i32 21, i32 3, i32 3, i32 0, i32 1}
// CHECK: ![[T2:.*]] = !{%dx.types.LinAlgMatrixC9M10N20U0S0 undef, i32 9, i32 10, i32 20, i32 0, i32 0}
// CHECK: ![[T3:.*]] = !{%dx.types.LinAlgMatrixC2M3N4U2S2 undef, i32 2, i32 3, i32 4, i32 2, i32 2}

// CHECKVAL-NOT: error: validation errors
// CHECKVAL-NOT: error: Named metadata 'dx.targetTypes' is unknown.
