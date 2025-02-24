SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupInclusiveMul_517979() {
  int4 arg_0 = (1).xxxx;
  int4 res = (WavePrefixProduct(arg_0) * arg_0);
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
