SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupExclusiveMul_4525a3() {
  int2 arg_0 = (1).xx;
  int2 res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_4525a3()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveMul_4525a3()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-37): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
