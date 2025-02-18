SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupExclusiveAdd_c08160() {
  int3 arg_0 = (1).xxx;
  int3 res = WavePrefixSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_c08160()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveAdd_c08160()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WavePrefixSum'


tint executable returned error: exit status 1
