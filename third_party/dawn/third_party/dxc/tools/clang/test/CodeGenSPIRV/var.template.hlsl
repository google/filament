// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpStore %is_same_v_0 %false
template <class, class>
static const bool is_same_v = false;

// CHECK: OpStore %is_same_v_1 %true
template <class T>
static const bool is_same_v<T, T> = true;

RWStructuredBuffer<bool> outs;

void main() {
// CHECK: [[inequal:%[0-9]+]] = OpLoad %bool %is_same_v_0
// CHECK:  [[outs_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outs %int_0 %uint_0
// CHECK: [[as_uint:%[0-9]+]] = OpSelect %uint [[inequal]] %uint_1 %uint_0
// CHECK:                         OpStore [[outs_0]] [[as_uint]]
  outs[0] = is_same_v<int, bool>;
// CHECK:   [[equal:%[0-9]+]] = OpLoad %bool %is_same_v_1
// CHECK:  [[outs_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outs %int_0 %uint_1
// CHECK: [[as_uint:%[0-9]+]] = OpSelect %uint [[equal]] %uint_1 %uint_0
// CHECK:                         OpStore [[outs_1]] [[as_uint]]
  outs[1] = is_same_v<int, int>;
}
