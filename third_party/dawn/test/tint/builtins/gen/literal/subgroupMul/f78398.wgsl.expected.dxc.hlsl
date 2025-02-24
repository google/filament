//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupMul_f78398() {
  float2 res = WaveActiveProduct((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_f78398()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupMul_f78398() {
  float2 res = WaveActiveProduct((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_f78398()));
  return;
}
