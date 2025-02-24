SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupInclusiveMul_359176() {
  uint3 res = (WavePrefixProduct((1u).xxx) * (1u).xxx);
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
<scrubbed_path>(4,16-42): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
