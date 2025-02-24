// RUN: %dxc -T cs_6_0 /Zi /O0 %s | FileCheck %s -check-prefix=OLD_TYPE
// RUN: %dxc -T cs_6_0 /Zi /O0 %s | FileCheck %s

// OLD_TYPE-NOT: !{{[0-9]+}} = !DIBasicType({{.+}}long

// CHECK-DAG: !{{[0-9]+}} = !DIDerivedType(tag: DW_TAG_typedef, name: "uint"
// CHECK-DAG: !{{[0-9]+}} = !DIBasicType(name: "int"

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

[numthreads(64, 1, 1)]
void main(int3 tid : SV_DispatchThreadId) {
  uint v = buf.Load<uint>(tid.y) * 2;
  int w = (int)v / 2;
  buf2.Store<int>(tid.z, w);
}

