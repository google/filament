SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupAdd_22d041() {
  int3 res = WaveActiveSum((int(1)).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
}

FXC validation failure:
<scrubbed_path>(4,14-40): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
