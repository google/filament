// RUN: %dxc -E main -T ps_6_6 %s | FileCheck %s

// call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 20491, i32 0 })
// CHECK: ; AnnotateHandle(res,props)  resource: globallycoherent RWByteAddressBuffer

RWByteAddressBuffer getBuf(uint i) {
  globallycoherent RWByteAddressBuffer buf = ResourceDescriptorHeap[i];
  return buf;
}

float4 main(uint i:I) : SV_Target {
  return asfloat(getBuf(i).Load(i));
}
