//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupInclusiveAdd_f8906d() {
  float2 arg_0 = (1.0f).xx;
  float2 v = arg_0;
  float2 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_f8906d()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupInclusiveAdd_f8906d() {
  float2 arg_0 = (1.0f).xx;
  float2 v = arg_0;
  float2 res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_f8906d()));
}

