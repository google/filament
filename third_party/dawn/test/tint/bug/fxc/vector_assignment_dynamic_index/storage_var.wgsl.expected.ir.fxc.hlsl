
cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
RWByteAddressBuffer v1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  v1.Store((0u + (min(i[0u].x, 2u) * 4u)), asuint(1.0f));
}

