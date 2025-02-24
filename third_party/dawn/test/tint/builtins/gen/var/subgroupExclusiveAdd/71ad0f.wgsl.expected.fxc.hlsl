SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupExclusiveAdd_71ad0f() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_71ad0f()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_71ad0f()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,16-35): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
