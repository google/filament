SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint2 subgroupInclusiveAdd_9bbcb0() {
  uint2 res = (WavePrefixSum((1u).xx) + (1u).xx);
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
<scrubbed_path>(4,16-37): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
