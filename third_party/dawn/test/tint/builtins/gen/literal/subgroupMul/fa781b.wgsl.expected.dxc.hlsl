//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupMul_fa781b() {
  uint3 res = WaveActiveProduct((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_fa781b()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupMul_fa781b() {
  uint3 res = WaveActiveProduct((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_fa781b()));
  return;
}
