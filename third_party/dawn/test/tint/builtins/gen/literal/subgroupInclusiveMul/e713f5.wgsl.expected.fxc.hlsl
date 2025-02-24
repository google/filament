SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupInclusiveMul_e713f5() {
  int2 res = (WavePrefixProduct((1).xx) * (1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
  return;
}
