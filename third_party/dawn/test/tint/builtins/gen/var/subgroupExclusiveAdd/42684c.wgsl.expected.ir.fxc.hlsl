SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupExclusiveAdd_42684c() {
  uint arg_0 = 1u;
  uint res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupExclusiveAdd_42684c());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupExclusiveAdd_42684c());
}

FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
