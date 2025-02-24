//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupExclusiveMul_f039f4() {
  uint3 res = WavePrefixProduct((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_f039f4()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupExclusiveMul_f039f4() {
  uint3 res = WavePrefixProduct((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_f039f4()));
  return;
}
