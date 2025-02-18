SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupExclusiveMul_d1d490() {
  uint2 res = WavePrefixProduct((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_d1d490()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_d1d490()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-40): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
