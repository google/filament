SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupInclusiveMul_dada1d() {
  uint2 arg_0 = (1u).xx;
  uint2 res = (WavePrefixProduct(arg_0) * arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_dada1d()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_dada1d()));
  return;
}
