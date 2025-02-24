SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupInclusiveMul_89437b() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupInclusiveMul_89437b());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupInclusiveMul_89437b());
}

FXC validation failure:
<scrubbed_path>(6,15-34): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
