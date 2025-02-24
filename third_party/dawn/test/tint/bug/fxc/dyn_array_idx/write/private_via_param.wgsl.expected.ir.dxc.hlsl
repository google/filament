struct S {
  int data[64];
};


cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};
RWByteAddressBuffer result : register(u1);
static S s = (S)0;
void x(inout S p) {
  uint v = min(uint(asint(ubo[0u].x)), 63u);
  p.data[v] = int(1);
}

[numthreads(1, 1, 1)]
void f() {
  x(s);
  result.Store(0u, asuint(s.data[3u]));
}

