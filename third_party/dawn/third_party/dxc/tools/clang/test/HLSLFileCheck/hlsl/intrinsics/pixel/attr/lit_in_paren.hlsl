// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: dx.op.evalSampleIndex.f32(i32 88, i32 0, i32 0, i8 0, i32 2)

float4 main(float4 a : A) : SV_TARGET {
  return EvaluateAttributeAtSample(a, (2));
}
