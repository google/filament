SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupMul_66c813() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = WaveActiveProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_66c813()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_66c813()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-39): error X3004: undeclared identifier 'WaveActiveProduct'


tint executable returned error: exit status 1
