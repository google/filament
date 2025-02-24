SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupMul_4f8ee6() {
  uint res = WaveActiveProduct(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupMul_4f8ee6());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupMul_4f8ee6());
}

FXC validation failure:
<scrubbed_path>(4,14-34): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
