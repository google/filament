SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupInclusiveAdd_8bbe75() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupInclusiveAdd_8bbe75());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupInclusiveAdd_8bbe75());
}

FXC validation failure:
<scrubbed_path>(6,16-31): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
