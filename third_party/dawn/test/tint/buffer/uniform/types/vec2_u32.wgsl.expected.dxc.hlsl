cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);

[numthreads(1, 1, 1)]
void main() {
  uint2 x = u[0].xy;
  s.Store2(0u, asuint(x));
  return;
}
