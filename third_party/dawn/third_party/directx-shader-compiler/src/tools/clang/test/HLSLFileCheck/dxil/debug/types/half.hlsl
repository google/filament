// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zi %s | FileCheck %s

// Test that debug information for half floats (16-bit type) is preserved.

// CHECK: %[[bufret:.*]] = call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16
// CHECK: %[[h:.*]] = extractvalue %dx.types.CBufRet.f16.8 %[[bufret]], 0
// CHECK: call void @llvm.dbg.value(metadata half %[[h]], i64 0, metadata ![[divar:.*]], metadata ![[diexpr:.*]]), !dbg

// CHECK: !DIBasicType(name: "half", size: 16, align: 16, encoding: DW_ATE_float)

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: ![[divar]] = !DILocalVariable(tag: DW_TAG_auto_variable, name: "h"
// CHECK-DAG: ![[diexpr]] = !DIExpression()

half cb_h;
float main() : OUT
{
    half h = cb_h;
    return (float)h;
}
