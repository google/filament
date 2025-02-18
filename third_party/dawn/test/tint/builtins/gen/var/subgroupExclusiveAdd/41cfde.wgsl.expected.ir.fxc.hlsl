SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupExclusiveAdd_41cfde() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_41cfde()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_41cfde()));
}

FXC validation failure:
<scrubbed_path>(5,16-35): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
