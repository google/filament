
cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};
RWByteAddressBuffer result : register(u2);
RWByteAddressBuffer ssbo : register(u1);
[numthreads(1, 1, 1)]
void f() {
  result.Store(0u, asuint(asint(ssbo.Load((0u + (min(uint(asint(ubo[0u].x)), 3u) * 4u))))));
}

