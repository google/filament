SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupMul_5a8c86() {
  int3 res = WaveActiveProduct((1).xxx);
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
<scrubbed_path>(4,14-39): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
