SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupAdd_fbc357() {
  uint4 res = WaveActiveSum((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupAdd_fbc357());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupAdd_fbc357());
}

FXC validation failure:
<scrubbed_path>(4,15-38): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
