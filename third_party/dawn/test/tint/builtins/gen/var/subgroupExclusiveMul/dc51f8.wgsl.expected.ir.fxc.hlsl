SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupExclusiveMul_dc51f8() {
  uint arg_0 = 1u;
  uint res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupExclusiveMul_dc51f8());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupExclusiveMul_dc51f8());
}

FXC validation failure:
<scrubbed_path>(5,14-37): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
