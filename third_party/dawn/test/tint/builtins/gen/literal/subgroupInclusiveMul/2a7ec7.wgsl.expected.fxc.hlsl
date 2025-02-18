SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupInclusiveMul_2a7ec7() {
  float res = (WavePrefixProduct(1.0f) * 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_2a7ec7()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_2a7ec7()));
  return;
}
