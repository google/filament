SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupInclusiveAdd_df692b() {
  float res = (WavePrefixSum(1.0f) + 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_df692b()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveAdd_df692b()));
}

FXC validation failure:
<scrubbed_path>(4,16-34): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
