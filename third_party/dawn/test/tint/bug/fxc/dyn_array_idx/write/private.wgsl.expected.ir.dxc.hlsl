struct S {
  int data[64];
};


cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};
RWByteAddressBuffer result : register(u1);
static S s = (S)0;
[numthreads(1, 1, 1)]
void f() {
  uint v = min(uint(asint(ubo[0u].x)), 63u);
  s.data[v] = int(1);
  result.Store(0u, asuint(s.data[3u]));
}

