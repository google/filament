// RUN: %dxc -T cs_6_5 -enable-16bit-types /Zi /O0 %s | FileCheck %s -check-prefix=SHORT
// RUN: %dxc -T cs_6_5 -enable-16bit-types /Zi /O0 %s | FileCheck %s

// SHORT-NOT: !{{[0-9]+}} = !DIBasicType(name: "short"
// SHORT-NOT: !{{[0-9]+}} = !DIBasicType(name: "unsigned short"

// CHECK-DAG: !{{[0-9]+}} = !DIBasicType(name: "uint16_t"
// CHECK-DAG: !{{[0-9]+}} = !DIBasicType(name: "int16_t"

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

[numthreads(64, 1, 1)]
void main(int3 tid : SV_DispatchThreadId) {
  uint16_t3x3 mat = buf.Load<uint16_t3x3>(tid.x);
  uint16_t3 v = buf.Load<uint16_t3>(tid.y);
  uint16_t3 w = mul(mat, v);
  int16_t3 ret = int16_t3(w.x, w.y, w.z);
  buf2.Store<int16_t3>(tid.z, ret);
}

