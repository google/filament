SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupInclusiveMul_9a54ec() {
  int res = (WavePrefixProduct(1) * 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_9a54ec()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_9a54ec()));
  return;
}
