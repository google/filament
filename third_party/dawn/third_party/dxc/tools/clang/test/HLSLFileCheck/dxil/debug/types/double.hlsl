// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Test that debug information for doubles (64-bit type) is preserved.

// CHECK: %[[bufret:.*]] = call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64
// CHECK: %[[d:.*]] = extractvalue %dx.types.CBufRet.f64 %[[bufret]], 0
// CHECK: call void @llvm.dbg.value(metadata double %[[d]], i64 0, metadata ![[divar:.*]], metadata ![[diexpr:.*]]), !dbg

// CHECK: !DIBasicType(name: "double", size: 64, align: 64, encoding: DW_ATE_float)

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: ![[divar]] = !DILocalVariable(tag: DW_TAG_auto_variable, name: "d"
// CHECK-DAG: ![[diexpr]] = !DIExpression()

double cb_d;
float main() : OUT
{
    double d = cb_d;
    return (float)d;
}
