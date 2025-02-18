SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupInclusiveMul_359176() {
  uint3 res = (WavePrefixProduct((1u).xxx) * (1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_359176()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_359176()));
  return;
}
