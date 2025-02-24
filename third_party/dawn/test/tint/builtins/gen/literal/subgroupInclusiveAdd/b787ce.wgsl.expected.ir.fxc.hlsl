SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupInclusiveAdd_b787ce() {
  float3 res = (WavePrefixSum((1.0f).xxx) + (1.0f).xxx);
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
<scrubbed_path>(4,17-41): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
