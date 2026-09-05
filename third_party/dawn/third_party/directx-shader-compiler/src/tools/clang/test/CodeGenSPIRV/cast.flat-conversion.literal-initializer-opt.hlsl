// RUN: %dxc -T ps_6_0 -E main %s -spirv | FileCheck %s

struct S {
  float f1;
  float f2;
};

struct T {
  int32_t i;
  int32_t j;
};

RWStructuredBuffer<S> float_output;
RWStructuredBuffer<T> int_output;

// CHECK-NOT: OpCapability Int64
// CHECK-NOT: OpCapability Float64

// CHECK-DAG: [[s:%[0-9]+]] = OpConstantComposite %S %float_3_40282321e_37 %float_3_40282321e_37
// CHECK-DAG: [[t:%[0-9]+]] = OpConstantComposite %T %int_1 %int_1

void main() {
// The literals should initially be interpreted as 64-bits, but fold to 32-bit values.
// The then 64-bit capabilities should be removed. Spir-v opt does not current fold OpFConvert.
// This should be updated when it does.
// CHECK: OpStore {{%[^ ]+}} [[s]]
  float_output[0] = (S)(3.402823466e+37);
// CHECK: OpStore {{%[^ ]+}} [[t]]
  int_output[0] = (T)(0x100000000+1);
}
