//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupExclusiveAdd_71ad0f() {
  float4 res = WavePrefixSum((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_71ad0f()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupExclusiveAdd_71ad0f() {
  float4 res = WavePrefixSum((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_71ad0f()));
  return;
}
