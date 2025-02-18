SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupExclusiveAdd_42684c() {
  uint res = WavePrefixSum(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_42684c()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_42684c()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,14-30): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
