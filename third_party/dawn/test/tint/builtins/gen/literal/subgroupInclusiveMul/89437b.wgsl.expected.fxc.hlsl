SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupInclusiveMul_89437b() {
  uint res = (WavePrefixProduct(1u) * 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_89437b()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_89437b()));
  return;
}
