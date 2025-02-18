//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupInclusiveAdd_367caa() {
  float4 arg_0 = (1.0f).xxxx;
  float4 v = arg_0;
  float4 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_367caa()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupInclusiveAdd_367caa() {
  float4 arg_0 = (1.0f).xxxx;
  float4 v = arg_0;
  float4 res = (WavePrefixSum(v) + v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_367caa()));
}

