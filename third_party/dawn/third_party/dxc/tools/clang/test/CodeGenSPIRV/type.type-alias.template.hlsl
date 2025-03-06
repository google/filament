// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

template<class T, T val>
struct integral_constant {
    static const T value = val;
};

template <bool val>
using bool_constant = integral_constant<bool, val>;

bool main(): SV_Target {
// CHECK: OpStore %value %true
// CHECK: %tru = OpVariable %_ptr_Function_integral_constant Function
  bool_constant<true> tru;

// CHECK: [[value:%[0-9]+]] = OpLoad %bool %value
// CHECK: OpReturnValue [[value]]
  return tru.value;
}
