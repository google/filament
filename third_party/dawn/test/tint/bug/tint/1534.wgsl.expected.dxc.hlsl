cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
RWByteAddressBuffer j : register(u1);

[numthreads(1, 1, 1)]
void main() {
  uint l = dot(i[0].xyz, i[0].xyz);
  j.Store(0u, asuint(i[0].x));
  return;
}
