//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupExclusiveAdd_4c8024() {
  float2 res = WavePrefixSum((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_4c8024()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupExclusiveAdd_4c8024() {
  float2 res = WavePrefixSum((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_4c8024()));
}

