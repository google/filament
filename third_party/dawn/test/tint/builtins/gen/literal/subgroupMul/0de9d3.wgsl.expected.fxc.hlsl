SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupMul_0de9d3() {
  float res = WaveActiveProduct(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-37): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
