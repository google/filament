// RUN: %dxc -T lib_6_6 %s | %FileCheck %s
// RUN: %dxc -T lib_6_6 -Od %s | %FileCheck %s
// RUN: %dxc -T lib_6_6 -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI
// RUN: %dxc -T lib_6_6 -Od -Zi %s | %FileCheck %s -check-prefixes=CHECK,CHECKZI

// CHECK: Note: shader requires additional functionality:
// CHECK: Resource descriptor heap indexing

// Make sure each entry get 2 createHandleFromHeap.
// CHECK:define void
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %{{.*}}, i1 false, i1 false)
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %{{.*}}, i1 false, i1 false)
// CHECK:define void
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %{{.*}}, i1 false, i1 false)
// CHECK:call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218, i32 %{{.*}}, i1 false, i1 false)

uint ID;
static const RWBuffer<float>           g_result      = ResourceDescriptorHeap[ID];
static ByteAddressBuffer         g_rawBuf      = ResourceDescriptorHeap[ID+1];
static float x = ID + 3;

// TODO: support array.
//  static Buffer<float> g_bufs[2] = {ResourceDescriptorHeap[ID+2], ResourceDescriptorHeap[ID+3]};

[shader("compute")]
[NumThreads(1, 1, 1)]
[RootSignature("RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | SAMPLER_HEAP_DIRECTLY_INDEXED), RootConstants(num32BitConstants=1, b0)")]
void csmain(uint ix : SV_GroupIndex)
{
  g_result[ix] = g_rawBuf.Load<float>(ix);// + g_bufs[0].Load(ix);
}
// export foo to make sure init function not removed.
export float foo(uint i) {
  return x + i;
}

[shader("compute")]
[NumThreads(1, 1, 1)]
[RootSignature("RootFlags(CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | SAMPLER_HEAP_DIRECTLY_INDEXED), RootConstants(num32BitConstants=1, b0)")]
void csmain2(uint ix : SV_GroupIndex)
{
  g_result[ix] = g_rawBuf.Load<float>(ix+ID);
}

// Exclude quoted source file (see readme)
// CHECKZI-LABEL: {{!"[^"]*\\0A[^"]*"}}
