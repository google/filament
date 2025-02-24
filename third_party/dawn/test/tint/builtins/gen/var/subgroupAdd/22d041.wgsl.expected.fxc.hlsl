SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupAdd_22d041() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
