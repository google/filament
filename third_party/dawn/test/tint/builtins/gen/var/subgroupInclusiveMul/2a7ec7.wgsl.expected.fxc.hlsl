SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupInclusiveMul_2a7ec7() {
  float arg_0 = 1.0f;
  float res = (WavePrefixProduct(arg_0) * arg_0);
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
