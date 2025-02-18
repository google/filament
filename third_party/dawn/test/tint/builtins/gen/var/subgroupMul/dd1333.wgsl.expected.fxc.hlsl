SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupMul_dd1333() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_dd1333()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_dd1333()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-38): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
