SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupInclusiveMul_1cdf5c() {
  uint4 res = (WavePrefixProduct((1u).xxxx) * (1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupInclusiveMul_1cdf5c());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupInclusiveMul_1cdf5c());
}

FXC validation failure:
<scrubbed_path>(4,16-43): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
