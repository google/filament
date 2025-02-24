// RUN: %dxc -HV 2021 -T cs_6_7 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[static_var:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private_int Private
// CHECK: [[init_var:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private_bool Private %false
// CHECK: %test = OpFunction %int None
// CHECK: [[init:%[a-zA-Z0-9_]+]] = OpLoad %bool [[init_var]]
// CHECK: OpBranchConditional [[init]] [[init_done_bb:%[a-zA-Z0-9_]+]] [[do_init_bb:%[a-zA-Z0-9_]+]]
// CHECK: [[do_init_bb]] = OpLabel
// CHECK: OpStore [[static_var]] %int_20
// CHECK: OpStore [[init_var]] %true
// CHECK: OpBranch [[init_done_bb]]
// CHECK: [[init_done_bb]] = OpLabel
// CHECK: OpLoad %int [[static_var]]


template <typename R> R test(R x) {
    static R v = 20;
    return v * x;
}

[numthreads(32, 32, 1)] void main(uint2 threadId: SV_DispatchThreadID) {
    float x = test(10);
}

