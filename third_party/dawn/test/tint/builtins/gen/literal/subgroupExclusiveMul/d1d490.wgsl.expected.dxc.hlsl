//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupExclusiveMul_d1d490() {
  uint2 res = WavePrefixProduct((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_d1d490()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupExclusiveMul_d1d490() {
  uint2 res = WavePrefixProduct((1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_d1d490()));
  return;
}
