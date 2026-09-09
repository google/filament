// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: %{{.*}} = call %dx.types.LinAlgMatrixC4M5N4U1S2 @dx.op.linAlgFillMatrix.mC4M5N4U1S2.i32
  // CHECK-SAME: (i32 -2147483636, i32 {{.*}})  ; LinAlgFillMatrix(value)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC4M5N4U1S2*, i32)"
  // CHECK2-SAME: (i32 402, %dx.types.LinAlgMatrixC4M5N4U1S2* {{.*}}, i32 5),
  // Matrix<I32, 5, 4, B, ThreadGroup>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat1;
  __builtin_LinAlg_FillMatrix(mat1, 5);

  // CHECK: %{{.*}} = call %dx.types.LinAlgMatrixC5M8N4U0S1 @dx.op.linAlgFillMatrix.mC5M8N4U0S1.f32
  // CHECK-SAME: (i32 -2147483636, float {{.*}})  ; LinAlgFillMatrix(value)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC5M8N4U0S1*, float)"
  // CHECK2-SAME: (i32 402, %dx.types.LinAlgMatrixC5M8N4U0S1* {{.*}}, float 0x40091EB860000000)
  // Matrix<U32, 8, 4, A, Wave>
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(5, 8, 4, 0, 1)]] mat2;
  __builtin_LinAlg_FillMatrix(mat2, 3.14);


  // CHECK: %{{.*}} = call %dx.types.LinAlgMatrixC5M8N4U0S2 @dx.op.linAlgFillMatrix.mC5M8N4U0S2.f64
  // CHECK-SAME: (i32 -2147483636, double {{.*}})  ; LinAlgFillMatrix(value)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC5M8N4U0S2*, double)"
  // CHECK2-SAME: (i32 402, %dx.types.LinAlgMatrixC5M8N4U0S2* {{.*}}, double %{{.+}})
  // Matrix<U32, 8, 4, A, ThreadGroup>
  double dVal = 9.87;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(5, 8, 4, 0, 2)]] mat3;
  __builtin_LinAlg_FillMatrix(mat3, dVal);


  // CHECK: %{{.*}} = call %dx.types.LinAlgMatrixC5M4N4U1S1 @dx.op.linAlgFillMatrix.mC5M4N4U1S1.i64
  // CHECK-SAME: (i32 -2147483636, i64 {{.*}})  ; LinAlgFillMatrix(value)

  // CHECK2: call void @"dx.hl.op..void (i32, %dx.types.LinAlgMatrixC5M4N4U1S1*, i64)"
  // CHECK2-SAME: (i32 402, %dx.types.LinAlgMatrixC5M4N4U1S1* {{.*}}, i64 %{{.+}})
  // Matrix<U32, 4, 4, B, Wave>
  int64_t i64Val = 12345;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(5, 4, 4, 1, 1)]] mat4;
  __builtin_LinAlg_FillMatrix(mat4, i64Val);
}
