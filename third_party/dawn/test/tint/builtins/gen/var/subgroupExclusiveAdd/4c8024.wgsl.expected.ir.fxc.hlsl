SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupExclusiveAdd_4c8024() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_4c8024()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_4c8024()));
}

FXC validation failure:
<scrubbed_path>(5,16-35): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
