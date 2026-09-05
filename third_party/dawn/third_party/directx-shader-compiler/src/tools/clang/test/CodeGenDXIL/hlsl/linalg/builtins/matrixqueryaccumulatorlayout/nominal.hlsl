// REQUIRES: dxil-1-10
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=CHECK2

[numthreads(1,1,1)]
void main() {
  // CHECK-LABEL: define void @main()

  // CHECK: call i32 @dx.op.linAlgMatrixQueryAccumulatorLayout(i32 -2147483626)  ; LinAlgMatrixQueryAccumulatorLayout()

  // CHECK2: call i32 @"dx.hl.op..i32 (i32)"(i32 414)
  uint layout = __builtin_LinAlg_MatrixQueryAccumulatorLayout();
}
