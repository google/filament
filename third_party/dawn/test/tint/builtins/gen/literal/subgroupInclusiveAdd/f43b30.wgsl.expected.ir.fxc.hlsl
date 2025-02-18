SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupInclusiveAdd_f43b30() {
  uint3 res = (WavePrefixSum((1u).xxx) + (1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupInclusiveAdd_f43b30());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupInclusiveAdd_f43b30());
}

FXC validation failure:
<scrubbed_path>(4,16-38): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
