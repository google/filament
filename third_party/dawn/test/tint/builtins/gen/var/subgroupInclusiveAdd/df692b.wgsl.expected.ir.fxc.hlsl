SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupInclusiveAdd_df692b() {
  float arg_0 = 1.0f;
  float v = arg_0;
  float res = (WavePrefixSum(v) + v);
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
<scrubbed_path>(6,16-31): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
