SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupExclusiveAdd_967e38() {
  float res = WavePrefixSum(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_967e38()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_967e38()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-33): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
