// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

template <class, class>
static const bool is_same_v = false;

template <class T>
static const bool is_same_v<T, T> = true;

RWStructuredBuffer<bool> outs;

void main() {
// CHECK:  [[outs_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outs %int_0 %uint_0
// CHECK: [[as_uint:%[0-9]+]] = OpSelect %uint %false %uint_1 %uint_0
// CHECK:                         OpStore [[outs_0]] [[as_uint]]
  outs[0] = is_same_v<int, bool>;
// CHECK:  [[outs_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %outs %int_0 %uint_1
// CHECK: [[as_uint:%[0-9]+]] = OpSelect %uint %true %uint_1 %uint_0
// CHECK:                         OpStore [[outs_1]] [[as_uint]]
  outs[1] = is_same_v<int, int>;
}
