// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -HV 202x -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -HV 202x -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(4,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: %{{.*}} = call <4 x i32> @dx.op.linAlgConvert.v4i32.v4f32
  // CHECK-SAME: (i32 -2147483618, <4 x float> <float 9.000000e+00, float 8.000000e+00, float 7.000000e+00, float 6.000000e+00>, i32 1, i32 2)
  // CHECK-SAME: ; LinAlgConvert(inputVector,inputInterpretation,outputInterpretation)

  // CHECK2: call void @"dx.hl.op..void (i32, <4 x i32>*, <4 x float>, i32, i32)"
  // CHECK2-SAME: (i32 422, <4 x i32>* %result1, <4 x float> %{{.*}}, i32 1, i32 2)
  float4 vec1 = {9.0, 8.0, 7.0, 6.0};
  int4 result1;
  __builtin_LinAlg_Convert(result1, vec1, 1, 2);

  // CHECK: %{{.*}} = call <4 x i64> @dx.op.linAlgConvert.v4i64.v4f64
  // CHECK-SAME: (i32 -2147483618, <4 x double> <double 9.000000e+00, double 8.000000e+00, double 7.000000e+00, double 6.000000e+00>, i32 1, i32 2)
  // CHECK-SAME: ; LinAlgConvert(inputVector,inputInterpretation,outputInterpretation)

  // CHECK2: call void @"dx.hl.op..void (i32, <4 x i64>*, <4 x double>, i32, i32)"
  // CHECK2-SAME: (i32 422, <4 x i64>* %result2, <4 x double> %{{.*}}, i32 1, i32 2)
  double4 vec2 = {9.0, 8.0, 7.0, 6.0};
  vector<int64_t, 4> result2;
  __builtin_LinAlg_Convert(result2, vec2, 1, 2);
}
