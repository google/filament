SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupMul_5a8c86() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveActiveProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_5a8c86()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_5a8c86()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-37): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
