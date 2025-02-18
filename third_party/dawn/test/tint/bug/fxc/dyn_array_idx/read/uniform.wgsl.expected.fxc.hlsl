cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[5];
};

RWByteAddressBuffer result : register(u2);

[numthreads(1, 1, 1)]
void f() {
  const uint scalar_offset = ((16u * min(uint(asint(ubo[4].x)), 3u))) / 4;
  result.Store(0u, asuint(asint(ubo[scalar_offset / 4][scalar_offset % 4])));
  return;
}
