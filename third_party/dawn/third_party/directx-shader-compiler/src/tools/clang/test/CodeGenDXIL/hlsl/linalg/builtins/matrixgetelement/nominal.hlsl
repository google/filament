// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(4, 5, 4, 1, 2)]] mat;

  // CHECK: call i32 @dx.op.linAlgMatrixGetElement.i32.mC4M5N4U1S2(i32 -2147483630,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 0)  
  // CHECK-SAME: ; LinAlgMatrixGetElement(matrix,threadLocalIndex)

  // CHECK2: call void @"dx.hl.op..void (i32, i32*, %dx.types.LinAlgMatrixC4M5N4U1S2, i32)"
  // CHECK2-SAME: (i32 404, i32* %elem1, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 0)
  uint elem1;
  __builtin_LinAlg_FillMatrix(mat, 1);
  __builtin_LinAlg_MatrixGetElement(elem1, mat, 0);

  // CHECK: call float @dx.op.linAlgMatrixGetElement.f32.mC4M5N4U1S2(i32 -2147483630,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1)  
  // CHECK-SAME: ; LinAlgMatrixGetElement(matrix,threadLocalIndex)

  // CHECK2: call void @"dx.hl.op..void (i32, float*, %dx.types.LinAlgMatrixC4M5N4U1S2, i32)"
  // CHECK2-SAME: (i32 404, float* %elem2, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1)
  float elem2;
  __builtin_LinAlg_MatrixGetElement(elem2, mat, 1);

  // CHECK: call double @dx.op.linAlgMatrixGetElement.f64.mC4M5N4U1S2(i32 -2147483630,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1)  
  // CHECK-SAME: ; LinAlgMatrixGetElement(matrix,threadLocalIndex)

  // CHECK2: call void @"dx.hl.op..void (i32, double*, %dx.types.LinAlgMatrixC4M5N4U1S2, i32)"
  // CHECK2-SAME: (i32 404, double* %elem3, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1)
  double elem3;
  __builtin_LinAlg_MatrixGetElement(elem3, mat, 1);


  // CHECK: call i64 @dx.op.linAlgMatrixGetElement.i64.mC4M5N4U1S2(i32 -2147483630,
  // CHECK-SAME: %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1)  
  // CHECK-SAME: ; LinAlgMatrixGetElement(matrix,threadLocalIndex)

  // CHECK2: call void @"dx.hl.op..void (i32, i64*, %dx.types.LinAlgMatrixC4M5N4U1S2, i32)"
  // CHECK2-SAME: (i32 404, i64* %elem4, %dx.types.LinAlgMatrixC4M5N4U1S2 %{{.*}}, i32 1)
  int64_t elem4;
  __builtin_LinAlg_MatrixGetElement(elem4, mat, 1);
}
