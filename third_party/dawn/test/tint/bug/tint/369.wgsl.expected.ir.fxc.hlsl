
ByteAddressBuffer SSBO : register(t0);
cbuffer cbuffer_UBO : register(b0) {
  uint4 UBO[1];
};
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

