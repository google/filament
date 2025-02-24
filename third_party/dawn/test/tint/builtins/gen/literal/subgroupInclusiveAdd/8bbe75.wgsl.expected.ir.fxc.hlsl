SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupInclusiveAdd_8bbe75() {
  uint4 res = (WavePrefixSum((1u).xxxx) + (1u).xxxx);
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
<scrubbed_path>(4,16-39): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
