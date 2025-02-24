SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupMul_0de9d3() {
  float arg_0 = 1.0f;
  float res = WaveActiveProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMul_0de9d3()));
}

FXC validation failure:
<scrubbed_path>(5,15-38): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
