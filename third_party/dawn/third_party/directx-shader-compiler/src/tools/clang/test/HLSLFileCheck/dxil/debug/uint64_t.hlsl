// RUN: %dxc -T cs_6_0 /Zi /O0 %s | FileCheck %s -check-prefix=OLD_TYPE
// RUN: %dxc -T cs_6_0 /Zi /O0 %s | FileCheck %s

// OLD_TYPE-NOT: !{{[0-9]+}} = !DIBasicType({{.+}}long long

// CHECK-DAG: !{{[0-9]+}} = !DIBasicType(name: "uint64_t"
// CHECK-DAG: !{{[0-9]+}} = !DIBasicType(name: "int64_t"

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

[numthreads(64, 1, 1)]
void main(int3 tid : SV_DispatchThreadId) {
  uint64_t v = buf.Load<uint64_t>(tid.y) * 2;
  int64_t w = (int64_t)v / 3;
  buf2.Store<int64_t>(tid.z, w);
}

