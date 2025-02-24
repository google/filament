// RUN: %dxc -T ps_6_6 %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Od %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI
// RUN: %dxc -T ps_6_6 -Od -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI

// CHECK: Note: shader requires additional functionality:
// CHECK: Resource descriptor heap indexing
// CHECK: Sampler descriptor heap indexing


//CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 0, i1 false, i1 false)
//CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 2, i32 1033 })
//CHECK-SAME: resource: Texture2D<4xF32>
//CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 0, i1 true, i1 false)
//CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 14, i32 0 })
//CHECK-SAME: resource: SamplerState


[RootSignature("RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED|SAMPLER_HEAP_DIRECTLY_INDEXED)")]
float4 main(float2 c:C) : SV_Target {

  Texture2D t = ResourceDescriptorHeap[0];
  SamplerState s = SamplerDescriptorHeap[0];
  return t.Sample(s, c);

}

// Exclude quoted source file (see readme)
// CHECKZI-LABEL: {{!"[^"]*\\0A[^"]*"}}
