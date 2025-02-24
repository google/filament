
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
[numthreads(1, 1, 1)]
void main() {
  int4 x = asint(u[0u]);
  s.Store4(0u, asuint(x));
}

