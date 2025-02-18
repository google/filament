SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMul_fa781b() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveActiveProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupMul_fa781b());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupMul_fa781b());
}

FXC validation failure:
<scrubbed_path>(5,15-38): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
