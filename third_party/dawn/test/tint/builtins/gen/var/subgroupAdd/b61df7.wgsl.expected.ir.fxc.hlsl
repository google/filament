SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupAdd_b61df7() {
  uint arg_0 = 1u;
  uint res = WaveActiveSum(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupAdd_b61df7());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupAdd_b61df7());
}

FXC validation failure:
<scrubbed_path>(5,14-33): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
