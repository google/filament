SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupInclusiveMul_517979() {
  int4 res = (WavePrefixProduct((1).xxxx) * (1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_517979()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_517979()));
  return;
}
