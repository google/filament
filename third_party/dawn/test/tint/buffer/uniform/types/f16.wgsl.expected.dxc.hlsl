cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);

[numthreads(1, 1, 1)]
void main() {
  float16_t x = float16_t(f16tof32(((u[0].x) & 0xFFFF)));
  s.Store<float16_t>(0u, x);
  return;
}
