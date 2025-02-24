// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK-NOT: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn HelperInvocation

float4 main() : SV_Target {
    float ret = 1.0;

    if (IsHelperLane()) ret = 2.0;

    return ret;
}
// CHECK: [[HelperInvocation:%[0-9]+]] = OpIsHelperInvocationEXT %bool
// CHECK: OpBranchConditional [[HelperInvocation]]
