
cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[5];
};
RWByteAddressBuffer result : register(u2);
[numthreads(1, 1, 1)]
void f() {
  uint v = (16u * min(uint(asint(ubo[4u].x)), 3u));
  result.Store(0u, asuint(asint(ubo[(v / 16u)][((v % 16u) / 4u)])));
}

