// RUN: %dxc -O3 -T lib_6_8 -HV 2021 -spirv -fcgl -fspv-target-env=universal1.5 -enable-16bit-types -Wno-gnu-static-float-init %s | FileCheck %s

template <typename S> struct Trait;
template <> struct Trait<half> {
  using type = half;
  static const uint32_t size = 2;
  static const half value = 1.0;
};

uint32_t get_size() { return Trait<half>::size; }
float cvt(half x) { return float(x); }

// CHECK-LABEL: %testcase = OpFunction %float
export float testcase(int x) {
  if (x == 2) {
    // CHECK: OpReturnValue %float_2
    return float(Trait<half>::size);
  } else if (x == 4) {
    // CHECK: OpStore %param_var_x %half_0x1p_0
    // CHECK-NEXT: OpFunctionCall %float %cvt %param_var_x
    return cvt(Trait<half>::value);
  } else if (x == 8) {
    return get_size();
  }
  return 0.f;
}

// CHECK-LABEL: %get_size = OpFunction %uint
// CHECK: OpReturnValue %uint_2
