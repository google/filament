SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupInclusiveAdd_b787ce() {
  float3 arg_0 = (1.0f).xxx;
  float3 v = arg_0;
  float3 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_b787ce()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveAdd_b787ce()));
}

FXC validation failure:
<scrubbed_path>(6,17-32): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
