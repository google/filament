SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupInclusiveMul_359176() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupInclusiveMul_359176());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupInclusiveMul_359176());
}

FXC validation failure:
<scrubbed_path>(6,16-35): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
