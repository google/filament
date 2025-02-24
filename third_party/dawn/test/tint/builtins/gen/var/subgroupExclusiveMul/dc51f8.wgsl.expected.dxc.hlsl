//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupExclusiveMul_dc51f8() {
  uint arg_0 = 1u;
  uint res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_dc51f8()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupExclusiveMul_dc51f8() {
  uint arg_0 = 1u;
  uint res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_dc51f8()));
  return;
}
