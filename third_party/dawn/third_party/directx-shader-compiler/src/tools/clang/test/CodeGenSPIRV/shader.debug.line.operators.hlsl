// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fspv-debug=vulkan -no-warnings -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK:      [[src:%[0-9]+]] = OpExtInst %void %1 DebugSource [[file]]

static int a, b, c;

void main() {
// CHECK:      DebugLine [[src]] %uint_11 %uint_11 %uint_11 %uint_15
// CHECK-NEXT: OpIAdd %int
  int d = a + b;

// CHECK:      DebugLine [[src]] %uint_15 %uint_15 %uint_7 %uint_11
// CHECK-NEXT: OpIMul %int
  c = a * b;

// CHECK:      DebugLine [[src]] %uint_19 %uint_19 %uint_21 %uint_25
// CHECK-NEXT: OpISub %int
  /* comment */ c = a - b;

// CHECK:      DebugLine [[src]] %uint_23 %uint_23 %uint_7 %uint_11
// CHECK-NEXT: OpSDiv %int
  c = a / b;

// CHECK:      DebugLine [[src]] %uint_27 %uint_27 %uint_8 %uint_12
// CHECK-NEXT: OpSLessThan %bool
  c = (a < b);

// CHECK:      DebugLine [[src]] %uint_31 %uint_31 %uint_7 %uint_11
// CHECK-NEXT: OpSGreaterThan %bool
  c = a > b;

// CHECK:      DebugLine [[src]] %uint_35 %uint_35 %uint_7 %uint_14
// CHECK-NEXT: OpLogicalAnd %bool
  c = (a) && b;

// CHECK:      DebugLine [[src]] %uint_39 %uint_39 %uint_3 %uint_8
// CHECK-NEXT: OpLogicalOr %bool
  a || b;

// CHECK: DebugLine [[src]] %uint_43 %uint_43 %uint_3 %uint_8
// CHECK: OpShiftLeftLogical %int
  a << b;

// CHECK: DebugLine [[src]] %uint_47 %uint_47 %uint_7 %uint_12
// CHECK: OpShiftRightArithmetic %int
  c = a >> b;

// CHECK:      DebugLine [[src]] %uint_51 %uint_51 %uint_7 %uint_11
// CHECK-NEXT: OpBitwiseAnd %int
  c = a & b;

// CHECK:      DebugLine [[src]] %uint_55 %uint_55 %uint_7 %uint_8
// CHECK-NEXT: OpNot %int
  c = ~b;

// CHECK:      DebugLine [[src]] %uint_59 %uint_59 %uint_7 %uint_11
// CHECK-NEXT: OpBitwiseXor %int
  c = a ^ b;

// CHECK:      DebugLine [[src]] %uint_67 %uint_67 %uint_3 %uint_7
// CHECK-NEXT: OpIAdd %int
// CHECK:      DebugLine [[src]] %uint_67 %uint_67 %uint_11 %uint_12
// CHECK-NEXT: OpNot %int
// CHECK-NEXT: DebugLine [[src]] %uint_67 %uint_67 %uint_3 %uint_12
// CHECK-NEXT: OpBitwiseXor %int
  c + a ^ ~b;

// CHECK:      DebugLine [[src]] %uint_71 %uint_71 %uint_3 %uint_5
// CHECK:      OpIAdd %int
  ++a;

// CHECK:      DebugLine [[src]] %uint_75 %uint_75 %uint_3 %uint_4
// CHECK:      OpIAdd %int
  a++;

// CHECK:      DebugLine [[src]] %uint_79 %uint_79 %uint_3 %uint_4
// CHECK:      OpISub %int
  a--;

// CHECK:      DebugLine [[src]] %uint_83 %uint_83 %uint_3 %uint_5
// CHECK:      OpISub %int
  --a;

// CHECK: DebugLine [[src]] %uint_87 %uint_87 %uint_3 %uint_9
// CHECK: OpShiftLeftLogical %int
  a <<= 10;

// CHECK:      DebugLine [[src]] %uint_91 %uint_91 %uint_3 %uint_8
// CHECK-NEXT: OpISub %int
  a -= 10;

// CHECK:      DebugLine [[src]] %uint_95 %uint_95 %uint_3 %uint_8
// CHECK-NEXT: OpIMul %int
  a *= 10;

// CHECK:      DebugLine [[src]] %uint_103 %uint_103 %uint_13 %uint_17
// CHECK-NEXT: OpIAdd %int
// CHECK-NEXT: DebugLine [[src]] %uint_103 %uint_103 %uint_8 %uint_18
// CHECK-NEXT: OpIAdd %int
// CHECK:      DebugLine [[src]] %uint_103 %uint_103 %uint_3 %uint_18
// CHECK-NEXT: OpSDiv %int
  a /= d + (b + c);

// CHECK:      DebugLine [[src]] %uint_109 %uint_109 %uint_8 %uint_12
// CHECK-NEXT: OpSLessThan %bool
// CHECK-NEXT: DebugLine [[src]] %uint_109 %uint_109 %uint_7 %uint_18
// CHECK-NEXT: OpLogicalAnd %bool
  b = (a < c) && true;

// CHECK:      DebugLine [[src]] %uint_113 %uint_113 %uint_3 %uint_8
// CHECK-NEXT: OpIAdd %int
  a += c;

// CHECK:      DebugLine [[src]] %uint_121 %uint_121 %uint_11 %uint_17
// CHECK-NEXT: OpIMul %int %int_100
// CHECK:      DebugLine [[src]] %uint_121 %uint_121 %uint_22 %uint_27
// CHECK-NEXT: OpISub %int %int_20
// CHECK-NEXT: DebugLine [[src]] %uint_121 %uint_121 %uint_11 %uint_28
// CHECK-NEXT: OpSDiv %int
  d = a + 100 * b / (20 - c);
// CHECK-NEXT: DebugLine [[src]] %uint_121 %uint_121 %uint_7 %uint_28
// CHECK-NEXT: OpIAdd %int

  float2x2 m2x2f;
  int2x2 m2x2i;

// CHECK:      DebugLine [[src]] %uint_132 %uint_132 %uint_11 %uint_15
// CHECK-NEXT: OpMatrixTimesScalar %mat2v2float
// CHECK:      DebugLine %35 %uint_132 %uint_132 %uint_11 %uint_23
// CHECK-NEXT: OpFAdd %v2float
  m2x2f = 2 * m2x2f + m2x2i;

// CHECK:      DebugLine [[src]] %uint_140 %uint_140 %uint_11 %uint_19
// CHECK-NEXT: OpFMul %v2float
// CHECK:      DebugLine [[src]] %uint_140 %uint_140 %uint_11 %uint_19
// CHECK-NEXT: OpFMul %v2float
// CHECK-NEXT: OpCompositeConstruct %mat2v2float
// this line added intentionally to keep from changing lines numbers
  m2x2f = m2x2f * m2x2i;

  float4 v4f;
  int4 v4i;

// CHECK:      DebugLine [[src]] %uint_147 %uint_147 %uint_9 %uint_15
// CHECK-NEXT: OpFDiv %v4float
  v4i = v4f / v4i;

// CHECK:      DebugLine [[src]] %uint_155 %uint_155 %uint_11 %uint_19
// CHECK-NEXT: OpFMul %v2float
// CHECK:      DebugLine [[src]] %uint_155 %uint_155 %uint_11 %uint_19
// CHECK-NEXT: OpFMul %v2float
// CHECK-NEXT: OpCompositeConstruct %mat2v2float
// this line added intentionally to keep from changing lines numbers
  m2x2f = m2x2f * v4f;

// CHECK:      DebugLine [[src]] %uint_159 %uint_159 %uint_11 %uint_23
// CHECK-NEXT: OpMatrixTimesScalar %mat2v2float
  m2x2f = m2x2f * v4f.x;

// CHECK:      DebugLine [[src]] %uint_165 %uint_165 %uint_4 %uint_10
// CHECK-NEXT: OpIMul %v4int
// CHECK:      DebugLine [[src]] %uint_165 %uint_165 %uint_3 %uint_15
// CHECK-NEXT: OpFOrdLessThan %v4bool
  (v4i * a) < m2x2f;

// CHECK:      DebugLine [[src]] %uint_171 %uint_171 %uint_4 %uint_10
// CHECK-NEXT: OpSDiv %v4int
// CHECK:      DebugLine [[src]] %uint_171 %uint_171 %uint_3 %uint_16
// CHECK-NEXT: OpLogicalAnd %v4bool
  (v4i / a) && v4f;
}
