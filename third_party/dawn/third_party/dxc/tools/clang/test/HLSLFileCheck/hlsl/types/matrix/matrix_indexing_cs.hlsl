// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure matrix indexing works for struct buf.
// CHECK: @dx.op.bufferLoad.f32
// CHECK: @dx.op.bufferStore.f32

StructuredBuffer<matrix<float, 4, 4> > buf1;
RWBuffer<float4> buf2;

[RootSignature("DescriptorTable(SRV(t0), UAV(u0))")]
[numthreads(8, 8, 1)]
void main( uint3 tid : SV_DispatchThreadID) {
  buf2[tid.x] = buf1[tid.x][tid.y][tid.z];
}