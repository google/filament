// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -enable-16bit-types %s | FileCheck %s

using MyHandleT = __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(9, 4, 4, 0, 1)]];

class MyMatrix {
  MyHandleT handle;

  static MyMatrix Splat(float Val) {
    MyMatrix Result;
    __builtin_LinAlg_FillMatrix(Result.handle, Val);
    return Result;
  }
};

[numthreads(4, 4, 4)]
void main() {
  MyMatrix MatA = MyMatrix::Splat(1.0f);
}

// CHECK: call %dx.types.LinAlgMatrixC9M4N4U0S1 @dx.op.linAlgFillMatrix.mC9M4N4U0S1.f32(i32 -2147483636, float 1.000000e+00)  ; LinAlgFillMatrix(value)
// CHECK-NOT: @llvm.trap()
