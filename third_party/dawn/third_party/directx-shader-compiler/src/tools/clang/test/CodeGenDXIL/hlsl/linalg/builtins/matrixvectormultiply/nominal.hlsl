// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2 

ByteAddressBuffer inbuf;

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // Matrix<I32, 4, 4, A, Thread>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 4, 4, 0, 0)]] mat;
  __builtin_LinAlg_MatrixLoadFromDescriptor(mat, inbuf, 0, 0, 0, 128);
  float4 vec = {1,2,3,4};
  float4 result;

  // CHECK: call <4 x float> @dx.op.linAlgMatVecMul.v4f32.mC4M4N4U0S0.v4f32(i32 -2147483623,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M4N4U0S0 %{{.*}}, i1 true, <4 x float> <float 1.000000e+00, float 2.000000e+00,
  // CHECK-SAME: float 3.000000e+00, float 4.000000e+00>, i32 9)  ; LinAlgMatVecMul(matrix,isOutputSigned,inputVector,interpretation)

  // CHECK2: call void @"dx.hl.op..void (i32, <4 x float>*, %dx.types.LinAlgMatrixC4M4N4U0S0, i1, <4 x float>, i32)
  // CHECK2-SAME: "(i32 418, <4 x float>* %result, %dx.types.LinAlgMatrixC4M4N4U0S0 %{{.*}}, i1 true, <4 x float> %{{.*}}, i32 9)
  __builtin_LinAlg_MatrixVectorMultiply(result, mat, true, vec, 9);
}
