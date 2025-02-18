SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupAdd_fbc357() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_fbc357()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_fbc357()));
  return;
}
FXC validation failure:
<scrubbed_path>(5,15-34): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
