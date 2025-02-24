// RUN: %dxc -T ps_6_6 %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Od %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI
// RUN: %dxc -T ps_6_6 -Od -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI

// CHECK: Note: shader requires additional functionality:
// CHECK: Resource descriptor heap indexing

//CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 0, i1 false, i1 false)
//CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 13, i32 4 })
//CHECK-SAME: resource: CBuffer
//CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 1, i1 false, i1 false)
//CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 15, i32 4 })
//CHECK-SAME: resource: TBuffer
//CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{.*}}, i32 0)
//CHECK:call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32 68, %dx.types.Handle %{{.*}}, i32 0, i32 undef)

struct A {
  float a;
};

static ConstantBuffer<A> C= ResourceDescriptorHeap[0];

float4 main(float2 c:C) : SV_Target {

TextureBuffer<A> T= ResourceDescriptorHeap[1];

  return C.a * T.a;

}

// Exclude quoted source file (see readme)
// CHECKZI-LABEL: {{!"[^"]*\\0A[^"]*"}}
