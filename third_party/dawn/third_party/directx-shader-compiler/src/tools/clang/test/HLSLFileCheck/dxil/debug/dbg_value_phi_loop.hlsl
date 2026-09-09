// RUN: %dxc -E main -T ps_6_0 %s -Zi -O3 | FileCheck %s

// CHECK-LABEL: @main()

// CHECK: phi float [
// CHECK: phi float [
// CHECK: phi float [
// CHECK: phi float [
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float

// CHECK: phi float [
// CHECK: phi float [
// CHECK: phi float [
// CHECK: phi float [
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float
// CHECK: call void @llvm.dbg.value(metadata float

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

[RootSignature("")]
float4 main(int a : A) : SV_Target {
  float4 result = float4(1,1,1,1);
  for (int i = 0; i < a; i++)
    result *= 4;
  return result;
}

