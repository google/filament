// RUN: %dxc -T lib_6_9 %s | FileCheck %s
// REQUIRES: dxil-1-9

// Initializing a coherent resource from ResourceDescriptorHeap[] goes through a
// flat-conversion. Verify the coherence qualifier of the destination is carried
// through to the annotated handle properties in the generated DXIL.

RWBuffer<float> OutBuf : register(u0);

// CHECK: call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218
// CHECK: call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 20491, i32 0 })
// CHECK-SAME: resource: globallycoherent RWByteAddressBuffer
[shader("raygeneration")]
void glc_entry()
{
  globallycoherent RWByteAddressBuffer buf = ResourceDescriptorHeap[0];
  buf.Store(0, 0);
}

// CHECK: call %dx.types.Handle @dx.op.createHandleFromHeap(i32 218
// CHECK: call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 69642, i32 265 })
// CHECK-SAME: resource: reordercoherent RWTypedBuffer<F32>
[shader("raygeneration")]
void rc_entry()
{
  reordercoherent RWBuffer<float> buf = ResourceDescriptorHeap[1];
  OutBuf[0] = buf[0];
}
