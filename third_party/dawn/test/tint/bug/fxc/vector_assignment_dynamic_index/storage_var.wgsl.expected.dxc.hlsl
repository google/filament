cbuffer cbuffer_i : register(b0) {
  uint4 i[1];
};
RWByteAddressBuffer v1 : register(u1);

[numthreads(1, 1, 1)]
void main() {
  v1.Store((4u * min(i[0].x, 2u)), asuint(1.0f));
  return;
}
