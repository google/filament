// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Test that the debug information for the result of a texture load
// is preserved after scalarization and optims.

// CHECK: call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32
// CHECK: extractvalue %dx.types.CBufRet.i32
// CHECK-DAG: call void @llvm.dbg.value
// CHECK-DAG: extractvalue %dx.types.CBufRet.i32
// CHECK: call void @llvm.dbg.value
// CHECK: call void @dx.op.storeOutput.i32
// CHECK: call void @dx.op.storeOutput.i32

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

int2 cb;
int2 main() : OUT
{
    int2 result = cb;
    return result;
}
