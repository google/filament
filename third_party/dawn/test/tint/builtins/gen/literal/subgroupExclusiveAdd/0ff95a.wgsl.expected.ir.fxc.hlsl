SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupExclusiveAdd_0ff95a() {
  uint3 res = WavePrefixSum((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupExclusiveAdd_0ff95a());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupExclusiveAdd_0ff95a());
}

FXC validation failure:
<scrubbed_path>(4,15-37): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
