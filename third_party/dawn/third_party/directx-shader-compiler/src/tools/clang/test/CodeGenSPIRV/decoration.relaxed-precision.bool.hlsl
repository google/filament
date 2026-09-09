// RUN: %dxc -T cs_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// According to the SPIR-V spec (2.14. Relaxed Precision):
// The RelaxedPrecision decoration can be applied to:
// The Result <id> of an instruction that operates on numerical types, meaning
// the instruction is to operate at relaxed precision. The instruction's
// operands may also be truncated to the relaxed precision.
//
// In this example, (a > 0.0) comparison is operating on floats with relaxed
// precision, and should therefore have the decoration. However, the "any"
// intrinsic function is operating on a vector of booleans, and it should not
// be decorated.

// CHECK:     OpDecorate %a RelaxedPrecision
// CHECK:     OpDecorate %b RelaxedPrecision
// CHECK:     OpDecorate [[a:%[0-9]+]] RelaxedPrecision
// CHECK:     OpDecorate [[compare_op:%[0-9]+]] RelaxedPrecision
// CHECK:     OpDecorate [[b:%[0-9]+]] RelaxedPrecision
// CHECK:     OpDecorate [[compare_op_2:%[0-9]+]] RelaxedPrecision
//
// We should NOT have a decoration for the 'any' operation.
// We should NOT have a decoration for the '||' operation.
//
// CHECK-NOT: OpDecorate {{%[0-9]+}} RelaxedPrecision

// CHECK:            [[a]] = OpLoad %float %a
// CHECK:   [[compare_op]] = OpFOrdGreaterThan %bool [[a]] %float_0
// CHECK:            [[b]] = OpLoad %v2float %b
// CHECK: [[compare_op_2]] = OpFOrdGreaterThan %v2bool [[b]] {{%[0-9]+}}
// CHECK:  [[any_op:%[0-9]+]] = OpAny %bool %26
// CHECK:   [[or_op:%[0-9]+]] = OpLogicalOr %bool

RWBuffer<float2> Buf;

[numthreads(1, 1, 1)]
void main() {
  // Scalar
  min16float a;

  // Vector
  min16float2 b;

  bool x = (a > 0.0) || any(b > 0.0);
}
