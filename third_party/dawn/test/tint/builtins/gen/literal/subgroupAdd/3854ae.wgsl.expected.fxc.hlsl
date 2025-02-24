SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupAdd_3854ae() {
  float res = WaveActiveSum(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_3854ae()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_3854ae()));
  return;
}
FXC validation failure:
<scrubbed_path>(4,15-33): error X3004: undeclared identifier 'WaveActiveSum'


tint executable returned error: exit status 1
