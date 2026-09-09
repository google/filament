// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

template<class T, T val>
struct integral_constant {
    static const T value = val;
};

template <bool val>
using bool_constant = integral_constant<bool, val>;

// CHECK: %true = OpConstantTrue %bool
bool main(): SV_Target {
// CHECK: %tru = OpVariable %_ptr_Function_integral_constant Function
  bool_constant<true> tru;

// CHECK: OpReturnValue %true
  return tru.value;
}
