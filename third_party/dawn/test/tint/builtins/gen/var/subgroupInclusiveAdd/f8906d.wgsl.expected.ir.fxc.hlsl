SKIP: INVALID


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

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveAdd_f8906d()));
}

FXC validation failure:
<scrubbed_path>(6,17-32): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
