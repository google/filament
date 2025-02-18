SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupInclusiveMul_9a54ec() {
  int arg_0 = 1;
  int res = (WavePrefixProduct(arg_0) * arg_0);
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
