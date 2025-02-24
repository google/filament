SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupAdd_22d041() {
  int3 res = WaveActiveSum((1).xxx);
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
<scrubbed_path>(4,14-35): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
