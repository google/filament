SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAdd_ba53f9() {
  int res = WaveActiveSum(int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_ba53f9()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_ba53f9()));
}

FXC validation failure:
<scrubbed_path>(4,13-33): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
