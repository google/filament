SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupMul_dc672a() {
  uint2 res = WaveActiveProduct((1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupMul_dc672a());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupMul_dc672a());
}

FXC validation failure:
<scrubbed_path>(4,15-40): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
