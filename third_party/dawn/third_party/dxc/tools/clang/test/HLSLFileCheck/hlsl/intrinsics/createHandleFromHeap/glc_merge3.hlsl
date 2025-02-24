// RUN: %dxc -T ps_6_6 %s | %FileCheck %s

// Make sure only 1 annotate handle and mark glc.
// Make sure glc is set.
// CHECK:call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 20491
// CHECK-NOT:call %dx.types.Handle @dx.op.annotateHandle(

float get(RWByteAddressBuffer buf, uint i) {

  return asfloat(buf.Load(i));

}

 

float4 main(uint i:I) : SV_Target {

  globallycoherent RWByteAddressBuffer buf = ResourceDescriptorHeap[i];

  return get(buf, i);

}
