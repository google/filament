SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupInclusiveMul_dada1d() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupInclusiveMul_dada1d());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupInclusiveMul_dada1d());
}

FXC validation failure:
<scrubbed_path>(6,16-35): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
