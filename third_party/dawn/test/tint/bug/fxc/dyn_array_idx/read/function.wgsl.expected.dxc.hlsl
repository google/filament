cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};

struct S {
  int data[64];
};

RWByteAddressBuffer result : register(u1);

[numthreads(1, 1, 1)]
void f() {
  S s = (S)0;
  result.Store(0u, asuint(s.data[min(uint(asint(ubo[0].x)), 63u)]));
  return;
}
