SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupInclusiveAdd_9bbcb0() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 res = (WavePrefixSum(v) + v);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, subgroupInclusiveAdd_9bbcb0());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, subgroupInclusiveAdd_9bbcb0());
}

FXC validation failure:
<scrubbed_path>(6,16-31): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
