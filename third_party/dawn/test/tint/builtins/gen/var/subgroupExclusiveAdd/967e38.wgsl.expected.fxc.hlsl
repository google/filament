SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupExclusiveAdd_967e38() {
  float arg_0 = 1.0f;
  float res = WavePrefixSum(arg_0);
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
<scrubbed_path>(5,15-34): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
