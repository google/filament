cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);

[numthreads(1, 1, 1)]
void main() {
  float3 x = asfloat(u[0].xyz);
  s.Store3(0u, asuint(x));
  return;
}
