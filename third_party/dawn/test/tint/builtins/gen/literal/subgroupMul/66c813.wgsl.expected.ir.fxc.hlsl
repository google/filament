SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupMul_66c813() {
  float4 res = WaveActiveProduct((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_66c813()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_66c813()));
}

FXC validation failure:
<scrubbed_path>(4,16-45): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
