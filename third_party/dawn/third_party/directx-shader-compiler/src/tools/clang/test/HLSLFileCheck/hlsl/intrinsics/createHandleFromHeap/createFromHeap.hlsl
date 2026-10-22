// RUN: %dxc -T ps_6_6 %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Od %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI
// RUN: %dxc -T ps_6_6 -Od -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI

// CHECK: Note: shader requires additional functionality:
// CHECK: Resource descriptor heap indexing

// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %{{.*}}, i1 false, i1 false)
// CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 10, i32 265 })
// CHECK-SAME: resource: TypedBuffer<F32>

uint ID;
float main(uint i:I): SV_Target {
  Buffer<float> buf = ResourceDescriptorHeap[ID];
  return buf[i];
}

// Exclude quoted source file (see readme)
// CHECKZI-LABEL: {{!"[^"]*\\0A[^"]*"}}
