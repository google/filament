// RUN: %dxc -T ps_6_6 %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Od %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI
// RUN: %dxc -T ps_6_6 -Zi -Od %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI

// Make sure generate createHandleFromBinding for sm6.6.
// CHECK:call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)
// CHECK:call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)
// Make sure sampler and texture get correct annotateHandle.

// CHECK-DAG:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 2, i32 1033 })
// CHECK-SAME: resource: Texture2D<4xF32>
// CHECK-DAG:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 14, i32 0 })
// CHECK-SAME: resource: SamplerState

SamplerState samplers : register(s0);
SamplerState foo() { return samplers; }
Texture2D t0 : register(t0);
[RootSignature("DescriptorTable(SRV(t0)),DescriptorTable(Sampler(s0))")]
float4 main( float2 uv : TEXCOORD ) : SV_TARGET {
  float4 val = t0.Sample(foo(), uv);
  return val;
}

// Exclude quoted source file (see readme)
// CHECKZI-LABEL: {{!"[^"]*\\0A[^"]*"}}
