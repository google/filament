SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupInclusiveAdd_367caa() {
  float4 res = (WavePrefixSum((1.0f).xxxx) + (1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_367caa()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveAdd_367caa()));
}

FXC validation failure:
<scrubbed_path>(4,17-42): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
